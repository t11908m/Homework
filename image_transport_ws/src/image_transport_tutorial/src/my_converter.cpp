#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

static const std::string OPENCV_WINDOW = "Image window";

class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;

public:
  ImageConverter()
    : it_(nh_)
  {
    // Subscrive to input video feed and publish output video feed
    image_sub_ = it_.subscribe("/camera/image_raw", 1,
      &ImageConverter::imageCb, this);
    image_pub_ = it_.advertise("/image_converter/output_video", 1);

    cv::namedWindow(OPENCV_WINDOW);
  }

  ~ImageConverter()
  {
    cv::destroyWindow(OPENCV_WINDOW);
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    // Draw an example circle on the video stream
    if (cv_ptr->image.rows > 60 && cv_ptr->image.cols > 60)
     {
      cv::circle(cv_ptr->image, cv::Point(50, 50), 10, CV_RGB(255,0,0));
     }
     cv::cvtColor(cv_ptr->image, cv_ptr->image, CV_BGR2GRAY); 
     cv::threshold(cv_ptr->image, cv_ptr->image, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
     
     
     
     
    cv::Mat Labeling;
    cv::Mat Stats;
    cv::Mat cent;
    int nLab = cv::connectedComponentsWithStats(cv_ptr->image,Labeling,Stats,cent,8);  
        // ラベリング結果の描画色を決定
    std::vector<cv::Vec3b> colors(nLab);
    colors[0] = cv::Vec3b(0, 0, 0);
    for (int i = 1; i < nLab; ++i) {
        colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
    }
     // ラベリング結果の描画
    cv::Mat Dst(cv_ptr->image.size(), CV_8UC3);
    for (int i = 0; i < Dst.rows; ++i) {
        int *lb = Labeling.ptr<int>(i);
        cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
        for (int j = 0; j < Dst.cols; ++j) {
            pix[j] = colors[lb[j]];
        }
    }
    int area_2 =0;
    int ID =0;
    for(int i = 1; i < nLab; i++){
       int *param = Stats.ptr<int>(i);
       int area = param[4];
       if(area > area_2){
          area_2 = area;
          ID = i;
       }
    }
    
    int *param = Stats.ptr<int>(ID); 
    int x = param[0];
    int y = param[1];
    int height = param[3];
    int width = param[2];
    cv::rectangle(Dst, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 2);
     



    // Update GUI Window
    cv::imshow(OPENCV_WINDOW, cv_ptr->image);
    cv::waitKey(3);

    // Output modified video stream
    image_pub_.publish(cv_ptr->toImageMsg());
  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  ros::spin();
  return 0;
}
