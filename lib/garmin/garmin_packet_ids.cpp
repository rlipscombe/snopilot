#include "config.h"
#include "trace.h"
#include "garmin_packet_ids.h"

std::string DescribePacketId(BYTE packet_id)
{
	// L000: Basic Packet IDs
	if (packet_id == Pid_Ack_Byte) return "Pid_Ack_Byte";
	if (packet_id == Pid_Nak_Byte) return "Pid_Nak_Byte";
	if (packet_id == Pid_Protocol_Array) return "Pid_Protocol_Array";
	if (packet_id == Pid_Product_Rqst) return "Pid_Product_Rqst";
	if (packet_id == Pid_Product_Data) return "Pid_Product_Data";

	// L001: Link Protocol 1 Packet IDs
	if (packet_id == Pid_Command_Data) return "Pid_Command_Data";
	if (packet_id == Pid_Xfer_Cmplt) return "Pid_Xfer_Cmplt";
	if (packet_id == Pid_Date_Time_Data) return "Pid_Date_Time_Data";
	if (packet_id == Pid_Position_Data) return "Pid_Position_Data";
	if (packet_id == Pid_Prx_Wpt_Data) return "Pid_Prx_Wpt_Data";
	if (packet_id == Pid_Records) return "Pid_Records";
	if (packet_id == Pid_Rte_Hdr) return "Pid_Rte_Hdr";
	if (packet_id == Pid_Rte_Wpt_Data) return "Pid_Rte_Wpt_Data";
	if (packet_id == Pid_Almanac_Data) return "Pid_Almanac_Data";
	if (packet_id == Pid_Trk_Data) return "Pid_Trk_Data";
	if (packet_id == Pid_Wpt_Data) return "Pid_Wpt_Data";
	if (packet_id == Pid_Pvt_Data) return "Pid_Pvt_Data";
	if (packet_id == Pid_Rte_Link_Data) return "Pid_Rte_Link_Data";
	if (packet_id == Pid_Trk_Hdr) return "Pid_Trk_Hdr";

	return "unknown";
}

std::string DescribeD108Color(BYTE color_id)
{
	if (color_id == D108_Color_Black) return "Black";
	if (color_id == D108_Color_Dark_Red) return "Dark_Red";
	if (color_id == D108_Color_Dark_Green) return "Dark_Green";
	if (color_id == D108_Color_Dark_Yellow) return "Dark_Yellow";
	if (color_id == D108_Color_Dark_Blue) return "Dark_Blue";
	if (color_id == D108_Color_Dark_Magenta) return "Dark_Magenta";
	if (color_id == D108_Color_Dark_Cyan) return "Dark_Cyan";
	if (color_id == D108_Color_Light_Gray) return "Light_Gray";
	if (color_id == D108_Color_Dark_Gray) return "Dark_Gray";
	if (color_id == D108_Color_Red) return "Red";
	if (color_id == D108_Color_Green) return "Green";
	if (color_id == D108_Color_Yellow) return "Yellow";
	if (color_id == D108_Color_Blue) return "Blue";
	if (color_id == D108_Color_Magenta) return "Magenta";
	if (color_id == D108_Color_Cyan) return "Cyan";
	if (color_id == D108_Color_White) return "White";
	if (color_id == D108_Color_Default_Color) return "Default_Color";

	return "unknown";
}

std::string DescribeD108WaypointClass(BYTE wpt_class)
{
	if (wpt_class == D108_Wpt_USER_WPT) return "User waypoint";
	if (wpt_class == D108_Wpt_AVTN_APT_WPT) return "Aviation Airport waypoint";
	if (wpt_class == D108_Wpt_AVTN_INT_WPT) return "Aviation Intersection waypoint";
	if (wpt_class == D108_Wpt_AVTN_NDB_WPT) return "Aviation NDB waypoint";
	if (wpt_class == D108_Wpt_AVTN_VOR_WPT) return "Aviation VOR waypoint";
	if (wpt_class == D108_Wpt_AVTN_ARWY_WPT) return "Aviation Airport Runway waypoint";
	if (wpt_class == D108_Wpt_AVTN_AINT_WPT) return "Aviation Airport Intersection";
	if (wpt_class == D108_Wpt_AVTN_ANDB_WPT) return "Aviation Airport NDB waypoint";
	if (wpt_class == D108_Wpt_MAP_PNT_WPT) return "Map Point waypoint";
	if (wpt_class == D108_Wpt_MAP_AREA_WPT) return "Map Area waypoint";
	if (wpt_class == D108_Wpt_MAP_INT_WPT) return "Map Intersection waypoint";
	if (wpt_class == D108_Wpt_MAP_ADRS_WPT) return "Map Address waypoint";
	if (wpt_class == D108_Wpt_MAP_LABEL_WPT) return "Map Label Waypoint";
	if (wpt_class == D108_Wpt_MAP_LINE_WPT) return "Map Line Waypoint";

	return "unknown";
}

std::string DescribeD103DisplayOptions(BYTE dspl)
{
	std::string r;

	if (dspl & D103_Dspl_dspl_name)
		r += "NAME ";
	if (dspl & D103_Dspl_dspl_none)
		r += "NONE ";
	if (dspl & D103_Dspl_dspl_cmnt)
		r += "COMMENT ";

	return r;
}

std::string DescribeSymbolType(int smbl)
{
	/*---------------------------------------------------------------
	Symbols for marine (group 0...0-8191...bits 15-13=000).
	---------------------------------------------------------------*/
	if (smbl == sym_anchor) return "white anchor symbol";
	if (smbl == sym_bell) return "white bell symbol";
	if (smbl == sym_diamond_grn) return "green diamond symbol";
	if (smbl == sym_diamond_red) return "red diamond symbol";
	if (smbl == sym_dive1) return "diver down flag 1";
	if (smbl == sym_dive2) return "diver down flag 2";
	if (smbl == sym_dollar) return "white dollar symbol";
	if (smbl == sym_fish) return "white fish symbol";
	if (smbl == sym_fuel) return "white fuel symbol";
	if (smbl == sym_horn) return "white horn symbol";
	if (smbl == sym_house) return "white house symbol";
	if (smbl == sym_knife) return "white knife & fork symbol";
	if (smbl == sym_light) return "white light symbol";
	if (smbl == sym_mug) return "white mug symbol";
	if (smbl == sym_skull) return "white skull and crossbones symbol";
	if (smbl == sym_square_grn) return "green square symbol";
	if (smbl == sym_square_red) return "red square symbol";
	if (smbl == sym_wbuoy) return "white buoy waypoint symbol";
	if (smbl == sym_wpt_dot) return "waypoint dot";
	if (smbl == sym_wreck) return "white wreck symbol";
	if (smbl == sym_null) return "null symbol (transparent)";
	if (smbl == sym_mob) return "man overboard symbol";
	/*------------------------------------------------------
	marine navaid symbols
	------------------------------------------------------*/
	if (smbl == sym_buoy_ambr) return "amber map buoy symbol";
	if (smbl == sym_buoy_blck) return "black map buoy symbol";
	if (smbl == sym_buoy_blue) return "blue map buoy symbol";
	if (smbl == sym_buoy_grn) return "green map buoy symbol";
	if (smbl == sym_buoy_grn_red) return "green/red map buoy symbol";
	if (smbl == sym_buoy_grn_wht) return "green/white map buoy symbol";
	if (smbl == sym_buoy_orng) return "orange map buoy symbol";
	if (smbl == sym_buoy_red) return "red map buoy symbol";
	if (smbl == sym_buoy_red_grn) return "red/green map buoy symbol";
	if (smbl == sym_buoy_red_wht) return "red/white map buoy symbol";
	if (smbl == sym_buoy_violet) return "violet map buoy symbol";
	if (smbl == sym_buoy_wht) return "white map buoy symbol";
	if (smbl == sym_buoy_wht_grn) return "white/green map buoy symbol";
	if (smbl == sym_buoy_wht_red) return "white/red map buoy symbol";
	if (smbl == sym_dot) return "white dot symbol";
	if (smbl == sym_rbcn) return "radio beacon symbol";
	/*------------------------------------------------------
	leave space for more navaids (up to 128 total)
	------------------------------------------------------*/
	if (smbl == sym_boat_ramp) return "boat ramp symbol";
	if (smbl == sym_camp) return "campground symbol";
	if (smbl == sym_restrooms) return "restrooms symbol";
	if (smbl == sym_showers) return "shower symbol";
	if (smbl == sym_drinking_wtr) return "drinking water symbol";
	if (smbl == sym_phone) return "telephone symbol";
	if (smbl == sym_1st_aid) return "first aid symbol";
	if (smbl == sym_info) return "information symbol";
	if (smbl == sym_parking) return "parking symbol";
	if (smbl == sym_park) return "park symbol";
	if (smbl == sym_picnic) return "picnic symbol";
	if (smbl == sym_scenic) return "scenic area symbol";
	if (smbl == sym_skiing) return "skiing symbol";
	if (smbl == sym_swimming) return "swimming symbol";
	if (smbl == sym_dam) return "dam symbol";
	if (smbl == sym_controlled) return "controlled area symbol";
	if (smbl == sym_danger) return "danger symbol";
	if (smbl == sym_restricted) return "restricted area symbol";
	if (smbl == sym_null_2) return "null symbol";
	if (smbl == sym_ball) return "ball symbol";
	if (smbl == sym_car) return "car symbol";
	if (smbl == sym_deer) return "deer symbol";
	if (smbl == sym_shpng_cart) return "shopping cart symbol";
	if (smbl == sym_lodging) return "lodging symbol";
	if (smbl == sym_mine) return "mine symbol";
	if (smbl == sym_trail_head) return "trail head symbol";
	if (smbl == sym_truck_stop) return "truck stop symbol";
	if (smbl == sym_user_exit) return "user exit symbol";
	if (smbl == sym_flag) return "flag symbol";
	if (smbl == sym_circle_x) return "circle with x in the center";
	/*---------------------------------------------------------------
	Symbols for land (group 1...8192-16383...bits 15-13=001).
	---------------------------------------------------------------*/
	if (smbl == sym_is_hwy) return "interstate hwy symbol";
	if (smbl == sym_us_hwy) return "us hwy symbol";
	if (smbl == sym_st_hwy) return "state hwy symbol";
	if (smbl == sym_mi_mrkr) return "mile marker symbol";
	if (smbl == sym_trcbck) return "TracBack (feet) symbol";
	if (smbl == sym_golf) return "golf symbol";
	if (smbl == sym_sml_cty) return "small city symbol";
	if (smbl == sym_med_cty) return "medium city symbol";
	if (smbl == sym_lrg_cty) return "large city symbol";
	if (smbl == sym_freeway) return "intl freeway hwy symbol";
	if (smbl == sym_ntl_hwy) return "intl national hwy symbol";
	if (smbl == sym_cap_cty) return "capitol city symbol (star)";
	if (smbl == sym_amuse_pk) return "amusement park symbol";
	if (smbl == sym_bowling) return "bowling symbol";
	if (smbl == sym_car_rental) return "car rental symbol";
	if (smbl == sym_car_repair) return "car repair symbol";
	if (smbl == sym_fastfood) return "fast food symbol";
	if (smbl == sym_fitness) return "fitness symbol";
	if (smbl == sym_movie) return "movie symbol";
	if (smbl == sym_museum) return "museum symbol";
	if (smbl == sym_pharmacy) return "pharmacy symbol";
	if (smbl == sym_pizza) return "pizza symbol";
	if (smbl == sym_post_ofc) return "post office symbol";
	if (smbl == sym_rv_park) return "RV park symbol";
	if (smbl == sym_school) return "school symbol";
	if (smbl == sym_stadium) return "stadium symbol";
	if (smbl == sym_store) return "dept. store symbol";
	if (smbl == sym_zoo) return "zoo symbol";
	if (smbl == sym_gas_plus) return "convenience store symbol";
	if (smbl == sym_faces) return "live theater symbol";
	if (smbl == sym_ramp_int) return "ramp intersection symbol";
	if (smbl == sym_st_int) return "street intersection symbol";
	if (smbl == sym_weigh_sttn) return "inspection/weigh station symbol";
	if (smbl == sym_toll_booth) return "toll booth symbol";
	if (smbl == sym_elev_pt) return "elevation point symbol";
	if (smbl == sym_ex_no_srvc) return "exit without services symbol";
	if (smbl == sym_geo_place_mm) return "Geographic place name, man-made";
	if (smbl == sym_geo_place_wtr) return "Geographic place name, water";
	if (smbl == sym_geo_place_lnd) return "Geographic place name, land";
	if (smbl == sym_bridge) return "bridge symbol";
	if (smbl == sym_building) return "building symbol";
	if (smbl == sym_cemetery) return "cemetery symbol";
	if (smbl == sym_church) return "church symbol";
	if (smbl == sym_civil) return "civil location symbol";
	if (smbl == sym_crossing) return "crossing symbol";
	if (smbl == sym_hist_town) return "historical town symbol";
	if (smbl == sym_levee) return "levee symbol";
	if (smbl == sym_military) return "military location symbol";
	if (smbl == sym_oil_field) return "oil field symbol";
	if (smbl == sym_tunnel) return "tunnel symbol";
	if (smbl == sym_beach) return "beach symbol";
	if (smbl == sym_forest) return "forest symbol";
	if (smbl == sym_summit) return "summit symbol";
	if (smbl == sym_lrg_ramp_int) return "large ramp intersection symbol";
	if (smbl == sym_lrg_ex_no_srvc) return "large exit without services smbl";
	if (smbl == sym_badge) return "police/official badge symbol";
	if (smbl == sym_cards) return "gambling/casino symbol";
	if (smbl == sym_snowski) return "snow skiing symbol";
	if (smbl == sym_iceskate) return "ice skating symbol";
	if (smbl == sym_wrecker) return "tow truck (wrecker) symbol";
	if (smbl == sym_border) return "border crossing (port of entry)";
	/*---------------------------------------------------------------
	Symbols for aviation (group 2...16383-24575...bits 15-13=010).
	---------------------------------------------------------------*/
	if (smbl == sym_airport) return "airport symbol";
	if (smbl == sym_int) return "intersection symbol";
	if (smbl == sym_ndb) return "non-directional beacon symbol";
	if (smbl == sym_vor) return "VHF omni-range symbol";
	if (smbl == sym_heliport) return "heliport symbol";
	if (smbl == sym_private) return "private field symbol";
	if (smbl == sym_soft_fld) return "soft field symbol";
	if (smbl == sym_tall_tower) return "tall tower symbol";
	if (smbl == sym_short_tower) return "short tower symbol";
	if (smbl == sym_glider) return "glider symbol";
	if (smbl == sym_ultralight) return "ultralight symbol";
	if (smbl == sym_parachute) return "parachute symbol";
	if (smbl == sym_vortac) return "VOR/TACAN symbol";
	if (smbl == sym_vordme) return "VOR-DME symbol";
	if (smbl == sym_faf) return "first approach fix";
	if (smbl == sym_lom) return "localizer outer marker";
	if (smbl == sym_map) return "missed approach point";
	if (smbl == sym_tacan) return "TACAN symbol";
	if (smbl == sym_seaplane) return "Seaplane Base";

	return "unknown";
}
