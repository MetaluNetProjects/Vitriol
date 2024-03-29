# import the necessary packages
from collections import deque
from imutils.video import VideoStream
import numpy as np
import argparse
import cv2
import imutils
import time

#picam = "http://raspberrypi:8080/?action=stream"

hsv = None
frame = None

import colortracker

greenLower = (89, 165, 104)
greenUpper = (129, 265, 204)
greenMid = (0, 255, 255)
pickindex = 0


# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video",
    help="path to the (optional) video file")
ap.add_argument("-b", "--buffer", type=int, default=64,
    help="max buffer size")
args = vars(ap.parse_args())


# if a video path was not supplied, grab the reference
# to the webcam
if not args.get("video", False):
    vs = VideoStream(src=0).start()

# otherwise, grab a reference to the video file
else:
    vs = cv2.VideoCapture(args["video"])
    #?vs.set(cv2.cv.CV_CAP_PROP_FPS, 1)

# allow the camera or video file to warm up
time.sleep(2.0)

pts = []
pts.append(deque(maxlen=args["buffer"]))
pts.append(deque(maxlen=args["buffer"]))
pts.append(deque(maxlen=args["buffer"]))

def pick_color(event,x,y,flags,param):
    global greenLower
    global greenUpper
    global greenMid
    if event == cv2.EVENT_LBUTTONDOWN:
        pixel = hsv[y,x]
        greenLower = pixel - (20, 50, 50)
        greenUpper = pixel + (20, 50, 50)
        pixelbrg = frame[y,x]
        greenMid = ( int (pixelbrg [ 0 ]), int (pixelbrg [ 1 ]), int (pixelbrg [ 2 ]))
        print(greenMid, greenLower, greenUpper)

# keep looping
while True:
    # grab the current frame
    frame = vs.read()

    # handle the frame from VideoCapture or VideoStream
    frame = frame[1] if args.get("video", False) else frame

    # if we are viewing a video and we did not grab a frame,
    # then we have reached the end of the video
    if frame is None:
        break

    # resize the frame, blur it, and convert it to the HSV
    # color space
    frame = imutils.resize(frame, width=800)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # construct a mask for the color "green", then perform
    # a series of dilations and erosions to remove any small
    # blobs left in the mask
    mask = cv2.inRange(hsv, greenLower, greenUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    # find contours in the mask and initialize the current
    # (x, y) center of the ball
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None

    # only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        # only proceed if the radius meets a minimum size
        if radius > 10:
            # draw the circle and centroid on the frame,
            # then update the list of tracked points
            #cv2.circle(frame, (int(x), int(y)), int(radius),
            #    (0, 255, 255), 2)
            cv2.circle(frame, (int(x), int(y)), int(radius),
                greenMid, 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)

    # update the points queue
    pts[pickindex].appendleft(center)

    # loop over the set of tracked points
    for i in range(1, len(pts[pickindex])):
        # if either of the tracked points are None, ignore
        # them
        if pts[pickindex][i - 1] is None or pts[pickindex][i] is None:
            continue

        # otherwise, compute the thickness of the line and
        # draw the connecting lines
        thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)
        cv2.line(frame, pts[pickindex][i - 1], pts[pickindex][i], (0, 0, 255), thickness)

    cv2.putText(frame, "Color {}".format(pickindex + 1), (20, 20),
        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    cv2.imshow("Frame", frame)
    cv2.imshow("Mask", mask)
    #CALLBACK FUNCTION
    cv2.setMouseCallback("Frame", pick_color)
    
    key = cv2.waitKey(1) & 0xFF

    if key == ord("p"):
        print(greenLower, greenUpper)
    elif key == ord("1"):
        pickindex = 0
    elif key == ord("2"):
        pickindex = 1
    elif key == ord("3"):
        pickindex = 2
    elif key == ord("4"):
        pickindex = 3
    # if the 'q' key is pressed, stop the loop
    elif key == ord("q"):
        break

# if we are not using a video file, stop the camera video stream
if not args.get("video", False):
    #vs.stop()
    0
# otherwise, release the camera
else:
    vs.release()

# close all windows
cv2.destroyAllWindows()
