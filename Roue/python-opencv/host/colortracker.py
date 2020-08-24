from collections import deque
import numpy as np
import argparse
import cv2
import imutils
from pyimagesearch.centroidtracker import CentroidTracker

class ColorTracker:
    def __init__(self, maxPoints=1):
        self.BGR = (0, 255, 255)
        self.HSVmin = (89, 165, 104)
        self.HSVmax = (129, 265, 204)
        self.pts = [] #deque(maxlen=64)
        self.mask = None
        self.maxPoints = maxPoints
        self.ct = CentroidTracker()

    def set_color(self, hsvcol, brgcol):
        self.HSVmin = hsvcol - (20, 50, 50)
        self.HSVmax = hsvcol + (20, 50, 50)
        self.BGR = ( int (brgcol [ 0 ]), int (brgcol [ 1 ]), int (brgcol [ 2 ]))
        print(self.BGR, self.HSVmin, self.HSVmax)

    def track(self, hsv, frame):
        # construct a mask for the color, then perform
        # a series of dilations and erosions to remove any small
        # blobs left in the mask
        mask = cv2.inRange(hsv, self.HSVmin, self.HSVmax)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)

        # find contours in the mask and initialize the current
        # (x, y) center of the ball
        cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
            cv2.CHAIN_APPROX_SIMPLE)
        cnts = imutils.grab_contours(cnts)
        self.pts = []

        # Find the index of the largest contours
        areas = [cv2.contourArea(c) for c in cnts]
        sortedcontours = np.argsort(areas)[::-1]
        for c_i in range(0, min(len(sortedcontours), self.maxPoints)):
        #for c_i in range(0, len(sortedcontours)):
            #if c_i == self.maxPoints:
                #break
            c = cnts[sortedcontours[c_i]]
        #for c in cnts:
            # find the largest contour in the mask, then use
            # it to compute the minimum enclosing circle and
            # centroid
            #c = max(cnts, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

            # only proceed if the radius meets a minimum size
            if radius > 10:
                # draw the circle and centroid on the frame,
                # then update the list of tracked points
                #cv2.circle(frame, (int(x), int(y)), int(radius),
                #    (0, 255, 255), 2)
                """cv2.circle(frame, (int(x), int(y)), int(radius),
                    self.BGR, 2)
                cv2.circle(frame, center, 5, (0, 0, 255), -1)"""
                self.pts.append(center)

        # update the points queue
        #self.pts.appendleft(center)
        self.objects = self.ct.update(self.pts)
        for (objectID, centroid) in self.objects.items():
        # draw both the ID of the object and the centroid of the
        # object on the output frame
            text = "ID {}".format(objectID)
            cv2.putText(frame, text, (centroid[0] - 10, centroid[1] - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            cv2.circle(frame, (centroid[0], centroid[1]), 4, (0, 255, 0), -1)

"""
        # loop over the set of tracked points
        for i in range(1, len(self.pts)):
            # if either of the tracked points are None, ignore
            # them
            if self.pts[i - 1] is None or self.pts[i] is None:
                continue

            # otherwise, compute the thickness of the line and
            # draw the connecting lines
            thickness = int(np.sqrt(64 / float(i + 1)) * 2.5)
            cv2.line(frame, self.pts[i - 1], self.pts[i], (0, 0, 255), thickness)
"""

