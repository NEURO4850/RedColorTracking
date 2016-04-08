#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <utility>

using namespace std;
using namespace cv;

class ColourTracker {
private:
    static const int LOWPASS_SIZE = 1;
    static const int MIN_AREA_TO_CARE_ABOUT = 1000;

    vector<double> xVals;
    vector<double> yVals;

    Scalar lowerThresholdValue;
    Scalar higherThresholdvalue;

    VideoCapture cap;

    Mat hsv, thresh;

    bool inDebug;

public:
    ColourTracker(Scalar lowThreshold = Scalar(0, 103, 59), Scalar highThreshold = Scalar(20, 255, 255), bool debug = true) {
        xVals = vector<double>(LOWPASS_SIZE, 0);
        yVals = vector<double>(LOWPASS_SIZE, 0);
        if (!setupCapture()) {
            exit(-1);
        }
        lowerThresholdValue = lowThreshold;
        higherThresholdvalue = highThreshold;

        inDebug = debug;
    }

    void trackColour() {
        while(true)
        {
            Mat frame;
            cap >> frame; // get a new frame from camera
            cvtColor(frame, hsv, CV_BGR2HSV);

            inRange(hsv, lowerThresholdValue, higherThresholdvalue, thresh);

            vector<Point> best = findBestContour();

            if (best.size() > 0) {
                Rect bRect = boundingRect(best);

                pair<double, double> xy = getXY(bRect, contourArea(best));

                if (inDebug) {
                    circle(frame, Point(xy.first + cap.get(CV_CAP_PROP_FRAME_WIDTH)/2.0, xy.second + cap.get(CV_CAP_PROP_FRAME_HEIGHT)/2.0), bRect.height/2, Scalar( 0, 255, 0), -1);
                    //drawContours(frame, thing, -1, Scalar(0, 255, 0), CV_FILLED);
                    imshow("Frame", frame);
                }

                cout << "XError: " << xy.first << " YError: " << xy.second << endl << endl;

                if(waitKey(30) >= 0)
                    break;
            }
        }
    }

    pair<double, double> getXY(Rect bRect, int area) {
        double cx = bRect.x + (bRect.width / 2);
        double cy = bRect.y + (bRect.height / 2);
        double XError = cx - cap.get(CV_CAP_PROP_FRAME_WIDTH)/2.0;
        double YError = cy - cap.get(CV_CAP_PROP_FRAME_HEIGHT)/2.0;

        if(area > MIN_AREA_TO_CARE_ABOUT) {
            xVals.erase(xVals.begin());
            xVals.push_back(XError);
            yVals.erase(yVals.begin());
            yVals.push_back(YError);
        }

        double x = accumulate(xVals.begin(), xVals.end(), 0.0)/xVals.size();
        double y = accumulate(yVals.begin(), yVals.end(), 0.0)/yVals.size();

        return make_pair(x, y);
    }

    vector<Point> findBestContour() {
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        findContours( thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

        double max_area = 0;
        vector<Point> best;
        for(unsigned int i=0;i<contours.size();i++) {
            double area = contourArea(contours[i]);
            if(area > max_area) {
                max_area = area;
                best = contours[i];
            }
        }

        return best;
    }

    bool setupCapture() {
        cap = VideoCapture(0);
        return cap.isOpened();
    }

};

int main(int, char**)
{   
    ColourTracker tr = ColourTracker();
    tr.trackColour();
    return 0;
}


