// ROS stuff
#include <rclcpp/rclcpp.hpp>
#include <rcutils/logging_macros.h>

#include "darknet_ros.hpp"

// Yolo stuff
#include <DarkHelp.hpp>

// Darknet messages
#include <perception_interfaces/msg/bounding_box.hpp>
#include <perception_interfaces/msg/bounding_boxes.hpp>

// Image transporter, messages, and OpenCV
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sensor_msgs/msg/image.h>

#include <array>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace darknet_ros {
class ObjectDetector : public rclcpp::Node {
  /// Subscriber to an image stream
  image_transport::Subscriber m_image_sub;
  /// Publisher of an image stream with annotated detections
  image_transport::Publisher m_image_pub;
  /// Bounding boxes publisher
  rclcpp::Publisher<perception_interfaces::msg::BoundingBoxes>::SharedPtr m_bbox_pub;

  /// YOLO Darknet detector
  std::string m_model_config_file;
  std::string m_model_weights_file;
  std::string m_model_names_file;
  float m_threshold;
  std::unique_ptr<DarkHelp> m_detector;

  void handle_image(const sensor_msgs::msg::Image::ConstSharedPtr& msg);

  template <typename T>
  void get_param(
      std::string param_name,
      T default_value,
      T& output_value,
      std::string_view log_info) {
    declare_parameter(param_name, rclcpp::ParameterValue(default_value));

    if (!get_parameter(param_name, output_value)) {
      RCLCPP_WARN_STREAM(
          get_logger(),
          "The parameter '" << param_name
                            << "' is not available or is not valid, "
                               "using the default value: "
                            << default_value);
    }

    if (!log_info.empty()) {
      RCLCPP_INFO_STREAM(get_logger(), log_info << output_value);
    }
  }

 public:
  ObjectDetector(const rclcpp::NodeOptions& options = rclcpp::NodeOptions()) :
      rclcpp::Node("object_detector", options) {
    RCLCPP_INFO(get_logger(), "************************************");
    RCLCPP_INFO(get_logger(), " Darknet Object Detection Component ");
    RCLCPP_INFO(get_logger(), "************************************");
    RCLCPP_INFO(get_logger(), " * namespace: %s", get_namespace());
    RCLCPP_INFO(get_logger(), " * node name: %s", get_name());
    RCLCPP_INFO(get_logger(), "************************************");

    m_image_sub = image_transport::create_subscription(
        this,
        DEFAULT_CAMERA_TOPIC,
        [this](const auto& img) -> void { this->handle_image(img); },
        "raw");
    m_image_pub = image_transport::create_publisher(this, "~/detection_image");

    m_bbox_pub = this->create_publisher<perception_interfaces::msg::BoundingBoxes>(
        "~/detections", DEFAULT_QOS);

    // Path to config file.
    declare_parameter("config_file");
    if (!get_parameter("config_file", m_model_config_file)) {
      RCLCPP_ERROR(get_logger(), "Model config file not given!");
    } else {
      RCLCPP_INFO_STREAM(get_logger(), " * Model Config: " << m_model_config_file);
    }
    // Path to weights file
    declare_parameter("weights_file");
    if (!get_parameter("weights_file", m_model_weights_file)) {
      RCLCPP_ERROR(get_logger(), "Model weights file not given!");
    } else {
      RCLCPP_INFO_STREAM(get_logger(), " * Model Weights: " << m_model_weights_file);
    }
    // Path to names file
    declare_parameter("names_file");
    if (!get_parameter("names_file", m_model_names_file)) {
      RCLCPP_ERROR(get_logger(), "Model names file not given!");
    } else {
      RCLCPP_INFO_STREAM(get_logger(), " * Model Names: " << m_model_names_file);
    }
    // Get object detector threshold
    declare_parameter("threshold", DEFAULT_DETECTION_THRESHOLD);
    get_parameter("threshold", m_threshold);
    RCLCPP_INFO_STREAM(get_logger(), " * Object Detection Threshold: " << m_threshold);

    m_detector = std::make_unique<DarkHelp>(
        m_model_config_file, m_model_weights_file, m_model_names_file);
  }
};

void ObjectDetector::handle_image(const sensor_msgs::msg::Image::ConstSharedPtr& msg) {
  cv_bridge::CvImagePtr cv_ptr;
  try {
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
  } catch (cv_bridge::Exception& e) {
    RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
    return;
  }
  // Get detections from darknet
  auto detections = m_detector->predict(cv_ptr->image);
  // Convert the detections to messages.
  auto bboxes = get_boxes(detections);
  // Draw the bounding boxes
  cv_ptr->image = m_detector->annotate();
  // Output modified video stream
  m_image_pub.publish(cv_ptr->toImageMsg());
  // Publish the bounding boxes
  auto bbox_msg           = perception_interfaces::msg::BoundingBoxes{};
  bbox_msg.header         = msg->header;
  bbox_msg.height         = msg->height;
  bbox_msg.width          = msg->width;
  bbox_msg.bounding_boxes = std::move(bboxes);
  m_bbox_pub->publish(bbox_msg);
}

}  // namespace darknet_ros

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<darknet_ros::ObjectDetector>();

  RCLCPP_INFO(
      rclcpp::get_logger(node->get_name()), "************************************");
  RCLCPP_INFO(rclcpp::get_logger(node->get_name()), " * Ready to detect objects");
  RCLCPP_INFO(
      rclcpp::get_logger(node->get_name()), "************************************");
  rclcpp::spin(node);
  rclcpp::shutdown();
}
