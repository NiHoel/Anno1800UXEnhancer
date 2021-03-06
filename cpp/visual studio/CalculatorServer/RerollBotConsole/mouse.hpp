#pragma once
#include <opencv2\core\types.hpp>


class mouse
{
public:
	mouse(const cv::Rect2i& desktop, const cv::Rect2i& window = cv::Rect2i());

	void click(const cv::Point2i& point);

	void click(const cv::Point2f& point);
	void move(const cv::Point2f& point);

	cv::Point2i get_ndc_position(const cv::Point2f relative_window_position) const;

private:
	cv::Rect2i desktop;
	cv::Rect2i window;

};

