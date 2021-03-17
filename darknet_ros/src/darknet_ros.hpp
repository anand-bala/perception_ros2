/// Some helper stuff for darknet_ros
#pragma once
#ifndef DARKNET_ROS_HPP
#define DARKNET_ROS_HPP

// Yolo stuff
#include <DarkHelp.hpp>

// Darknet messages
#include <perception_interfaces/msg/bounding_box.hpp>
#include <perception_interfaces/msg/bounding_boxes.hpp>

namespace darknet_ros
{
/// Default detection threshold for YOLO
constexpr float DEFAULT_DETECTION_THRESHOLD = 0.3;

constexpr auto DEFAULT_CAMERA_TOPIC = "camera";
constexpr auto DEFAULT_QOS = 1;

std::vector<perception_interfaces::msg::BoundingBox>
get_boxes(const DarkHelp::PredictionResults& bounding_boxes)
{
  std::vector<perception_interfaces::msg::BoundingBox> ret;
  for (const auto& box : bounding_boxes)
  {
    perception_interfaces::msg::BoundingBox msg;
    msg.probability = box.best_probability;
    msg.class_id = box.best_class;
    msg.class_label = box.name;
    msg.x = box.rect.x;
    msg.y = box.rect.y;
    msg.width = box.rect.width;
    msg.height = box.rect.height;
    msg.tracking_id = -1;  //< Not tracked.
    ret.push_back(msg);
  }
  return ret;
}

}  // namespace darknet_ros

#endif /* end of include guard: DARKNET_ROS_HPP */
