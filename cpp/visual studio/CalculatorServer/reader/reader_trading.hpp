#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>


namespace reader
{
	class image_recognition;
	struct item;

class trading_params
{
	friend class trading_menu;

	static const cv::Scalar background_marine_blue;
	static const cv::Scalar background_sand_dark;
	static const cv::Scalar background_sand_bright;
	static const cv::Scalar background_grey_bright;
	static const cv::Scalar background_grey_dark;
	static const cv::Scalar background_green_bright;
	static const cv::Scalar background_green_dark;
	static const cv::Scalar background_cargo_slot;
	static const cv::Scalar background_trading_menu;
	static const cv::Scalar frame_brown;
	static const cv::Scalar red_icon;

	static const cv::Rect2f size_offering;
	static const cv::Rect2f size_offering_price;
	static const cv::Rect2f size_offering_icon;
	static const cv::Rect2f size_icon;
	static const cv::Rect2f size_icon_small;

	static const unsigned int count_cols;
	static const unsigned int count_rows;

	static const cv::Rect2f pane_menu_offering;
	static const cv::Rect2f pane_menu_offering_with_counter;
	static const cv::Rect2f pane_menu_name;
	static const cv::Rect2f pane_menu_reroll;
	static const cv::Rect2f pane_menu_execute;
	static const cv::Rect2f pane_menu_title;
	static const cv::Rect2f pane_menu_ship_cargo;
	static const cv::Rect2f pane_menu_ship_sockets;
	static const cv::Rect2f pane_menu_available_items;

	static const cv::Rect2f pane_tooltip_reroll_heading;
	static const cv::Rect2f pane_tooltip_reroll_price;

	static const cv::Point2f pixel_ship_full;
	static const cv::Point2f pixel_background_sockets_color;
};

struct offering
{
	unsigned int index;
	cv::Rect2i box;
	unsigned int price;
	std::vector<std::shared_ptr<item>> item_candidates;

	bool operator==(const offering& other) const;
};

class trading_menu
{
public:
	trading_menu(image_recognition& recog);

	void update(const std::string& language, const cv::Mat& img);


	bool is_trading_menu_open() const;
	bool has_reroll() const;
	bool has_buy_limit() const;
	/*
	* Trading menue: Tests wheter the trade button can be clicked
	* Preview: Tests whether the open trade menu button is available
	*/
	bool can_buy() const;
	/*
	 * Trading menue: Tests wheter the item at the specified index can be added to the cart
	 * Returns false in all other cases
	 */
	bool can_buy(unsigned int index) const;
	bool can_buy(const offering& off) const;

	bool is_ship_full() const;

	/*
	* Returns all currently offered items
	* @param{abort_if_not_loaded} If one of the items is not fully rendered (i.e. only the background shown), 
	* the method returns an empty vector
	*/
	std::vector<offering> get_offerings(bool abort_if_not_loaded = false);
	std::vector<offering> get_capped_items() const;

	/*
	* Positions of buttons relative to the window
	*/
	cv::Rect2f get_execute_button() const;
	cv::Rect2f get_reroll_button() const;

	/*
	* Returns the modification of trading prices introduced
	* by capped itemes
	*/
	int get_price_modification() const;

	/**
	* Given an index from an @ref{offering} it returns 
	* the position of the corresponding item on the screenshot.
	* Distinguishes whether preview or trade menu is open.
	*/
	cv::Rect2i get_window_abs_location(unsigned int index) const;
	cv::Rect2f get_window_rel_location(unsigned int index) const;
	cv::Rect2i get_window_abs_location(const cv::Rect2i& roi_rel_location) const;
	cv::Rect2f get_window_rel_location(const cv::Rect2f& roi_rel_location) const;

	unsigned int get_open_trader() const;

	/*
	* Returns cost for next reroll
	* Returns 0 if reroll tooltip is not open
	*/
	unsigned int get_reroll_cost() const;

	unsigned int get_buy_limit() const;

private:
	image_recognition& recog;
	cv::Mat screenshot;
	std::map<unsigned int, cv::Mat> ship_items;
	std::map<unsigned int, std::vector<std::pair<int, cv::Mat>>> cached_prices;
	cv::Mat storage_icon;
	unsigned int window_width;

	unsigned int open_trader;
	unsigned int buy_limit;
	bool menu_open;
	bool buy_limited;
	
	int get_price(const cv::Mat& offering);
	
	/**
* Checks for active, price reducing items and returns whether @param{selling_price}
* and the reduced item price of @param{guid} match
*/
	bool check_price(unsigned int guid, unsigned int selling_price, int price_modification_percent = 0) const;

	cv::Rect2i get_roi_abs_location(unsigned int index) const;
	cv::Rect2f get_roi_rel_location(unsigned int index) const;

	cv::Rect2i to_abs_location(const cv::Rect2f& box) const;
};

}
