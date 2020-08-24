# import the necessary packages
from collections import deque
from imutils.video import VideoStream
import numpy as np
import argparse
import cv2
import imutils
import time
import colortracker as ct

#picam = "http://raspberrypi:8080/?action=stream"

hsv = None
frame = None

"""
greenLower = (89, 165, 104)
greenUpper = (129, 265, 204)
greenMid = (0, 255, 255)
"""
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

"""pts = []
pts.append(deque(maxlen=args["buffer"]))
pts.append(deque(maxlen=args["buffer"]))
pts.append(deque(maxlen=args["buffer"]))
"""

trackers = []
trackers.append(ct.ColorTracker(1));
trackers.append(ct.ColorTracker(3));

def pick_color(event,x,y,flags,param):
    if event == cv2.EVENT_LBUTTONDOWN:
        hsvcol = hsv[y,x]
        brgcol = frame[y,x]
        trackers[pickindex].set_color(hsvcol, brgcol)

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

    #trackers[0].track(hsv, frame)
    #trackers[1].track(hsv, frame)
    for tracker in trackers:
        tracker.track(hsv, frame)
    cv2.putText(frame, "Color {}".format(pickindex + 1), (20, 20),
        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    cv2.imshow("Frame", frame)
    #cv2.imshow("Mask", mask)
    #CALLBACK FUNCTION
    #cv2.setMouseCallback("Frame", trackers[0].pick_color, [hsv, frame])
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
