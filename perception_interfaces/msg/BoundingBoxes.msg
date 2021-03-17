# This defines a list of bounding boxes outputted by a perception algorithm for object
# detection and tracking.

std_msgs/Header header         # Header timestamp should be acquisition time of image
                               # Header frame_id should be optical frame of camera

uint32 height                  # image height, that is, number of rows
uint32 width                   # image width, that is, number of columns

BoundingBox[] bounding_boxes
