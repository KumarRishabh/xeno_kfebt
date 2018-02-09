#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <std_msgs/Int64MultiArray.h>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <bits/stdc++.h>
#include "kfebtracker.h"

using namespace cv;
using namespace std;

#define CAMERA 0

bool Detected;
bool mousePress;
bool rectOK;
Rect initRect;
KFebTracker tracker;
int flag;
int start_track;
void processVideo(Mat);
void initializeVideo(Mat);
void msrect(Mat);

static void OnDetect( const std_msgs::Int64MultiArray::ConstPtr& point){
    /*if( event != EVENT_LBUTTONDOWN || rectOK)
        return;

    if(mousePress){
        initRect.width = x - initRect.x;
        initRect.height = y - initRect.y;
        rectOK = true;
    }

    if(!mousePress){
        initRect.x = x;
        initRect.y = y;
        mousePress = true;
    }*/
    initRect.x = point->data[0];
    initRect.y = point->data[1];
    initRect.width = point->data[0] - 2*point->data[2];
    initRect.height = point->data[1] - 2*point->data[2];
}
void initializeVideo(Mat image) {
  // Interface
  Detected = false;
  rectOK = false;

  tracker.init("AKC");

  Rect region;

  bool run = 1;

  // cv::VideoCapture cam(CAMERA);

  imshow("result", image);
  // setMouseCallback("result", OnDetect, 0);
}
void msrect(Mat image)
{
      imshow("result", image);
      waitKey(100);

  while(!rectOK){
      waitKey(100);
  }

  tracker.initTrackers(image, initRect);
  flag=5;
}

void processVideo(Mat image){
      // Read image

      if(image.empty()){
            cout << "/* hmm kya hua run nahi hua kya */" << '\n';
      }

      // Report result
      Rect output = tracker.track(image);
      Mat saida = image.clone();
      rectangle(saida, output,Scalar(0,255,0), 2);
      imshow("result", saida);
      waitKey(1000);

  return ;
}

void call(const sensor_msgs::ImageConstPtr& msg){
while(ros::ok()){
  ROS_INFO("IMAGE RECIEVED\n");
  cv_bridge::CvImagePtr cv_ptr;

  // create Background Subtractor objects
  // pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach

  cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
  Mat img = cv_ptr->image;
  // update the background model

  if(start_track==0)
  {
    cout<<"get val"<<endl;
    cin>>start_track;
    cout <<"val = "<<start_track<<endl;
    imshow("hmm", img);
  }

  if(start_track==1)
  {

    if (flag==1)
    {
      initializeVideo(img);
      flag++;
    }
    
    while(!mousePress && flag !=1)
    {  
      msrect(img);
    }

    if (flag==5) 
    {
      /* code */
        processVideo(img);
    }

  }
  waitKey(10);
}
}


int main(int argc, char** argv){

  // Interface
  // Integrating ROS
  flag =1;
  start_track=0;
  ros::init(argc, argv, "object_track");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);
  ros::Subscriber sub = nh.subscribe("measurement", 1, OnDetect);
  ros::Rate loop_rate(10);
  // ros::Subscriber sub = nh.subscribe("state", 1, OnDetect);
  image_transport::Subscriber image_sub_ = it.subscribe("input", 1, call);

  ros::spin();

  return 0;
}
