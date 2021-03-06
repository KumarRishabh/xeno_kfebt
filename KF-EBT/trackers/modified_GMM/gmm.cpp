/**
 * @file bg_sub.cpp
 * @brief Background subtraction tutorial sample code
 * @author Domenico D. Bloisi
 */

//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace cv;
using namespace std;

// Global variables
Mat frame; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard; //input from keyboard
int get_h(int b,int g,int r);


/** Function Headers */
void help();
void processVideo(char* videoFilename);
void processImages(char* firstFrameFilename);

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use background subtraction methods provided by "  << endl
    << " OpenCV. You can process both videos (-vid) and images (-img)."             << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./bs {-vid <video filename>|-img <image filename>}"                         << endl
    << "for example: ./bs -vid video.avi"                                           << endl
    << "or: ./bs -img /data/images/1.png"                                           << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    //print help information
    help();

    //check for the input parameter correctness
    if(argc != 3) {
        cerr <<"Incorret input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }

    //create GUI windows
    namedWindow("Frame");
    namedWindow("FG Mask MOG 2");

    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach

    if(strcmp(argv[1], "-vid") == 0) {
        //input data coming from a video
        processVideo(argv[2]);
    }
    else if(strcmp(argv[1], "-img") == 0) {
        //input data coming from a sequence of images
        processImages(argv[2]);
    }
    else {
        //error in reading input parameters
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}

/**
 * @function processVideo
 */
void processVideo(char* videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    int Lth = 0, Hth = 0,size=3,h;
    createTrackbar("lower threshold", "red", &Lth, 255);
    createTrackbar("higher threshold", "red", &Hth, 255);
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        Mat img = frame.clone();
      //  createTrackbar("lower threshold", "red", &Lth, 255);
      //createTrackbar("higher threshold", "red", &Hth, 255);

        //update the background model
        pMOG2->apply(frame, fgMaskMOG2);
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks

        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard

        Mat img_hsv;
        cvtColor(img,img_hsv, CV_BGR2HSV);

        Mat red_buoy(frame.rows,frame.cols,CV_8UC1);
        Mat red_hsv(frame.rows,frame.cols,CV_8UC1);
        for (int i = 0; i < frame.rows; i++)
	      {
            for (int j = 0; j < frame.cols; j++)
            {
              //  h = get_h((int)img_hsv.at<Vec3b>(i,j)[0],(int)img_hsv.at<Vec3b>(i,j)[1],(int)img_hsv.at<Vec3b>(i,j)[2]);
                if((int)fgMaskMOG2.at<uchar>(i,j)==255 && (int)img_hsv.at<Vec3b>(i,j)[0] > 150)
                {
                    red_buoy.at<uchar>(i,j)=255;
                }
                else  red_buoy.at<uchar>(i,j)= 0 ;

                red_hsv.at<uchar>(i,j) = (int)img_hsv.at<Vec3b>(i,j)[0];
            }
        }

        imshow("red_buoy",red_buoy);
        imshow("red_hsv",red_hsv);
        imshow("img_hsv",img_hsv);
        GaussianBlur(red_buoy,red_buoy,Size(15,15),2,2);

      //  Mat buoy_contour(frame.rows,frame.cols,CV_8UC1);

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        findContours(red_buoy, contours, hierarchy, RETR_CCOMP,CHAIN_APPROX_SIMPLE);

      //  cout << contours.size() << '\n';
        if(contours.size() != 0)
        {


            /// Draw contours

            Mat drawing = Mat::zeros( red_buoy.size(), CV_8UC3 );
            Mat dst = Mat::zeros( red_buoy.size(), CV_8UC3 );

            int idx = 0, largestComp = 0;
            double maxArea = 0;

            vector<Point2f>center( contours.size() );
            vector<float>radius( contours.size() );
            vector<vector<Point> > contours_poly( contours.size() );

            for( ; idx >= 0; idx = hierarchy[idx][0] )
            {
                const vector<Point>& c = contours[idx];
                double area = fabs(contourArea(Mat(c)));
                if( area > maxArea )
                {
                    maxArea = area;
                    largestComp = idx;
                }
                approxPolyDP( Mat(contours[idx]), contours_poly[idx], 3, true );
                minEnclosingCircle( (Mat)contours_poly[idx], center[idx], radius[idx] );
            }
            Scalar color( 0, 0, 255 );
            drawContours( dst, contours, largestComp, color, FILLED, LINE_8, hierarchy );
            circle( drawing, center[largestComp], (int)radius[largestComp], color, 2, 8, 0 );
           imshow("contours",drawing);
        }
       /*
       int idx=0;
       for( ; idx >= 0; idx = hierarchy[idx][0] )
      {
          Scalar color( rand()&255, rand()&255, rand()&255 );
          drawContours( dst, contours, idx, color, CV_FILLED, 8, hierarchy );
      }*/






      //  Mat red_circle;
        //Canny(red_buoy, red_circle, Lth, Hth, size);
      //  imshow("red",red_circle);

        /*
        vector<Vec3f> circles;
        HoughCircles(red_circle,circles,CV_HOUGH_GRADIENT,1,red_buoy.rows/10,100,200,0,0);
        cout<<circles.size()<<endl;
        for( size_t i = 0 ;i <circles.size(); i++ )
        {
            Point center(cvRound(circles[i][0]),cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            circle(frame,center,3,Scalar(0,255,0),-1,8,0);
            circle(frame,center,radius,Scalar(0,0,255),3,8,0);
            cout<<i<<"hi"<<endl;
        }    */

        imshow("Frame", frame);

        keyboard = waitKey( 30 );
    }
    //delete capture object
    capture.release();
}


/**
 * @function processImages
 */
void processImages(char* fistFrameFilename) {
    //read the first file of the sequence
    frame = imread(fistFrameFilename);
    if(frame.empty()){
        //error in opening the first image
        cerr << "Unable to open first image frame: " << fistFrameFilename << endl;
        exit(EXIT_FAILURE);
    }
    //current image filename
    string fn(fistFrameFilename);
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //update the background model
        pMOG2->apply(frame, fgMaskMOG2);
        //get the frame number and write it on the current frame
        size_t index = fn.find_last_of("/");
        if(index == string::npos) {
            index = fn.find_last_of("\\");
        }
        size_t index2 = fn.find_last_of(".");
        string prefix = fn.substr(0,index+1);
        string suffix = fn.substr(index2);
        string frameNumberString = fn.substr(index+1, index2-index-1);
        istringstream iss(frameNumberString);
        int frameNumber = 0;
        iss >> frameNumber;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey( 30 );
        //search for the next image in the sequence
        ostringstream oss;
        oss << (frameNumber + 1);
        string nextFrameNumberString = oss.str();
        string nextFrameFilename = prefix + nextFrameNumberString + suffix;
        //read the next frame
        frame = imread(nextFrameFilename);
        if(frame.empty()){
            //error in opening the next image in the sequence
            cerr << "Unable to open image frame: " << nextFrameFilename << endl;
            exit(EXIT_FAILURE);
        }
        //update the path of the current frame
        fn.assign(nextFrameFilename);
    }
}
