/* garmin_packet_ids.h
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 *
 * Other references:
 *   Here's some (typically readable) perl which appears to do tracklog downloads:
 *   http://www.w-beer.de/pub/garman/garman
 *
 *   Here's Garmin-mode download, which may be quicker (binary)
 *   Horse's mouth: http://www.garmin.com/support/pdf/iop_spec.pdf (includes D300 and D301 (with altitude) record specs).
 *   3rd party accounts: http://www.abnormal.com/~thogard/gps/grmnprot.html
 *                       http://www.radio-active.net.au/web/gps/garmin.html
 */

#ifndef GARMIN_PACKET_IDS_H
#define GARMIN_PACKET_IDS_H 1

#include <string>
#include <vector>

// L000: Basic Packet IDs
const BYTE Pid_Ack_Byte = 6;
const BYTE Pid_Nak_Byte = 21;
const BYTE Pid_Protocol_Array = 253; /* may not be implemented in all products */
const BYTE Pid_Product_Rqst = 254;
const BYTE Pid_Product_Data = 255;

// L001: Link Protocol 1 Packet IDs
const BYTE Pid_Command_Data = 10;
const BYTE Pid_Xfer_Cmplt = 12;
const BYTE Pid_Date_Time_Data = 14;
const BYTE Pid_Position_Data = 17;
const BYTE Pid_Prx_Wpt_Data = 19;
const BYTE Pid_Records = 27;
const BYTE Pid_Rte_Hdr = 29;
const BYTE Pid_Rte_Wpt_Data = 30;
const BYTE Pid_Almanac_Data = 31;
const BYTE Pid_Trk_Data = 34;
const BYTE Pid_Wpt_Data = 35;
const BYTE Pid_Pvt_Data = 51;
const BYTE Pid_Rte_Link_Data = 98;
const BYTE Pid_Trk_Hdr = 99;

// A010: Device Command Protocol 1
const INT16 Cmnd_Abort_Transfer = 0; /* abort current transfer */
const INT16 Cmnd_Transfer_Alm = 1; /* transfer almanac */
const INT16 Cmnd_Transfer_Posn = 2; /* transfer position */
const INT16 Cmnd_Transfer_Prx = 3; /* transfer proximity waypoints */
const INT16 Cmnd_Transfer_Rte = 4; /* transfer routes */
const INT16 Cmnd_Transfer_Time = 5; /* transfer time */
const INT16 Cmnd_Transfer_Trk = 6; /* transfer track log */
const INT16 Cmnd_Transfer_Wpt = 7; /* transfer waypoints */
const INT16 Cmnd_Turn_Off_Pwr = 8; /* turn off power */
const INT16 Cmnd_Start_Pvt_Data = 49; /* start transmitting PVT data */
const INT16 Cmnd_Stop_Pvt_Data = 50; /* stop transmitting PVT data */

std::string DescribePacketId(BYTE packet_id);

/* Various packet data types
 * Note: none of these structures are sent to/received from the device directly.
 * That'd open up all kinds of alignment/padding problems.
 * They're only used on the PC side, and are composed using packet.ReadByte etc..
 */
struct Semicircle_Type
{
	long lat; /* latitude in semicircles */
	long lon; /* longitude in semicircles */
};

struct Records_Type
{
	int packet_count;
};

struct Product_Data_Type
{
	int product_id;
	int software_version;

	std::string product_description;

	typedef std::vector<std::string> strings_t;
	strings_t additional_strings;

	Product_Data_Type()
		: product_id(0), software_version(0)
	{
	}
};

struct D300_Trk_Point_Type
{
	Semicircle_Type posn; /* position */
	unsigned long time; /* time (seconds since 0:00 on Dec 31, 1989) */
	bool new_trk; /* new track segment? */
};

struct D301_Trk_Point_Type
{
	Semicircle_Type posn; /* position */
	unsigned long time; /* time (seconds since 0:00 on Dec 31, 1989) */
	float alt; /* altitude in meters */
	float dpth; /* depth in meters */
	bool new_trk; /* new track segment? */
};

struct D310_Trk_Hdr_Type
{
	bool dspl;
	BYTE color;
	std::string trk_ident;
};

// Essentially, these are VGA colours.
const BYTE D108_Color_Black = 0;
const BYTE D108_Color_Dark_Red = 1;
const BYTE D108_Color_Dark_Green = 2;
const BYTE D108_Color_Dark_Yellow = 3;
const BYTE D108_Color_Dark_Blue = 4;
const BYTE D108_Color_Dark_Magenta = 5;
const BYTE D108_Color_Dark_Cyan = 6;
const BYTE D108_Color_Light_Gray = 7;
const BYTE D108_Color_Dark_Gray = 8;
const BYTE D108_Color_Red = 9;
const BYTE D108_Color_Green = 10;
const BYTE D108_Color_Yellow = 11;
const BYTE D108_Color_Blue = 12;
const BYTE D108_Color_Magenta = 13;
const BYTE D108_Color_Cyan = 14;
const BYTE D108_Color_White = 15;
const BYTE D108_Color_Default_Color = 0xFF;

std::string DescribeD108Color(BYTE color_id);
inline std::string DescribeD310Color(BYTE color_id) { return DescribeD108Color(color_id); }

const BYTE D108_Wpt_USER_WPT = 0x00;
const BYTE D108_Wpt_AVTN_APT_WPT = 0x40;
const BYTE D108_Wpt_AVTN_INT_WPT = 0x41;
const BYTE D108_Wpt_AVTN_NDB_WPT = 0x42;
const BYTE D108_Wpt_AVTN_VOR_WPT = 0x43;
const BYTE D108_Wpt_AVTN_ARWY_WPT = 0x44;
const BYTE D108_Wpt_AVTN_AINT_WPT = 0x45;
const BYTE D108_Wpt_AVTN_ANDB_WPT = 0x46;
const BYTE D108_Wpt_MAP_PNT_WPT = 0x80;
const BYTE D108_Wpt_MAP_AREA_WPT = 0x81;
const BYTE D108_Wpt_MAP_INT_WPT = 0x82;
const BYTE D108_Wpt_MAP_ADRS_WPT = 0x83;
const BYTE D108_Wpt_MAP_LABEL_WPT = 0x84;
const BYTE D108_Wpt_MAP_LINE_WPT = 0x85;

std::string DescribeD108WaypointClass(BYTE wpt_class);

const BYTE D103_Dspl_dspl_name = 0;
const BYTE D103_Dspl_dspl_none = 1;
const BYTE D103_Dspl_dspl_cmnt = 2;

std::string DescribeD103DisplayOptions(BYTE dspl);

enum
{
	/*---------------------------------------------------------------
	Symbols for marine (group 0...0-8191...bits 15-13=000).
	---------------------------------------------------------------*/
	sym_anchor = 0, /* white anchor symbol */
	sym_bell = 1, /* white bell symbol */
	sym_diamond_grn = 2, /* green diamond symbol */
	sym_diamond_red = 3, /* red diamond symbol */
	sym_dive1 = 4, /* diver down flag 1 */
	sym_dive2 = 5, /* diver down flag 2 */
	sym_dollar = 6, /* white dollar symbol */
	sym_fish = 7, /* white fish symbol */
	sym_fuel = 8, /* white fuel symbol */
	sym_horn = 9, /* white horn symbol */
	sym_house = 10, /* white house symbol */
	sym_knife = 11, /* white knife & fork symbol */
	sym_light = 12, /* white light symbol */
	sym_mug = 13, /* white mug symbol */
	sym_skull = 14, /* white skull and crossbones symbol*/
	sym_square_grn = 15, /* green square symbol */
	sym_square_red = 16, /* red square symbol */
	sym_wbuoy = 17, /* white buoy waypoint symbol */
	sym_wpt_dot = 18, /* waypoint dot */
	sym_wreck = 19, /* white wreck symbol */
	sym_null = 20, /* null symbol (transparent) */
	sym_mob = 21, /* man overboard symbol */
	/*------------------------------------------------------
	marine navaid symbols
	------------------------------------------------------*/
	sym_buoy_ambr = 22, /* amber map buoy symbol */
	sym_buoy_blck = 23, /* black map buoy symbol */
	sym_buoy_blue = 24, /* blue map buoy symbol */
	sym_buoy_grn = 25, /* green map buoy symbol */
	sym_buoy_grn_red = 26, /* green/red map buoy symbol */
	sym_buoy_grn_wht = 27, /* green/white map buoy symbol */
	sym_buoy_orng = 28, /* orange map buoy symbol */
	sym_buoy_red = 29, /* red map buoy symbol */
	sym_buoy_red_grn = 30, /* red/green map buoy symbol */
	sym_buoy_red_wht = 31, /* red/white map buoy symbol */
	sym_buoy_violet = 32, /* violet map buoy symbol */
	sym_buoy_wht = 33, /* white map buoy symbol */
	sym_buoy_wht_grn = 34, /* white/green map buoy symbol */
	sym_buoy_wht_red = 35, /* white/red map buoy symbol */
	sym_dot = 36, /* white dot symbol */
	sym_rbcn = 37, /* radio beacon symbol */
	/*------------------------------------------------------
	leave space for more navaids (up to 128 total)
	------------------------------------------------------*/
	sym_boat_ramp = 150, /* boat ramp symbol */
	sym_camp = 151, /* campground symbol */
	sym_restrooms = 152, /* restrooms symbol */
	sym_showers = 153, /* shower symbol */
	sym_drinking_wtr = 154, /* drinking water symbol */
	sym_phone = 155, /* telephone symbol */
	sym_1st_aid = 156, /* first aid symbol */
	sym_info = 157, /* information symbol */
	sym_parking = 158, /* parking symbol */
	sym_park = 159, /* park symbol */
	sym_picnic = 160, /* picnic symbol */
	sym_scenic = 161, /* scenic area symbol */
	sym_skiing = 162, /* skiing symbol */
	sym_swimming = 163, /* swimming symbol */
	sym_dam = 164, /* dam symbol */
	sym_controlled = 165, /* controlled area symbol */
	sym_danger = 166, /* danger symbol */
	sym_restricted = 167, /* restricted area symbol */
	sym_null_2 = 168, /* null symbol */
	sym_ball = 169, /* ball symbol */
	sym_car = 170, /* car symbol */
	sym_deer = 171, /* deer symbol */
	sym_shpng_cart = 172, /* shopping cart symbol */
	sym_lodging = 173, /* lodging symbol */
	sym_mine = 174, /* mine symbol */
	sym_trail_head = 175, /* trail head symbol */
	sym_truck_stop = 176, /* truck stop symbol */
	sym_user_exit = 177, /* user exit symbol */
	sym_flag = 178, /* flag symbol */
	sym_circle_x = 179, /* circle with x in the center */
	/*---------------------------------------------------------------
	Symbols for land (group 1...8192-16383...bits 15-13=001).
	---------------------------------------------------------------*/
	sym_is_hwy = 8192, /* interstate hwy symbol */
	sym_us_hwy = 8193, /* us hwy symbol */
	sym_st_hwy = 8194, /* state hwy symbol */
	sym_mi_mrkr = 8195, /* mile marker symbol */
	sym_trcbck = 8196, /* TracBack (feet) symbol */
	sym_golf = 8197, /* golf symbol */
	sym_sml_cty = 8198, /* small city symbol */
	sym_med_cty = 8199, /* medium city symbol */
	sym_lrg_cty = 8200, /* large city symbol */
	sym_freeway = 8201, /* intl freeway hwy symbol */
	sym_ntl_hwy = 8202, /* intl national hwy symbol */
	sym_cap_cty = 8203, /* capitol city symbol (star) */
	sym_amuse_pk = 8204, /* amusement park symbol */
	sym_bowling = 8205, /* bowling symbol */
	sym_car_rental = 8206, /* car rental symbol */
	sym_car_repair = 8207, /* car repair symbol */
	sym_fastfood = 8208, /* fast food symbol */
	sym_fitness = 8209, /* fitness symbol */
	sym_movie = 8210, /* movie symbol */
	sym_museum = 8211, /* museum symbol */
	sym_pharmacy = 8212, /* pharmacy symbol */
	sym_pizza = 8213, /* pizza symbol */
	sym_post_ofc = 8214, /* post office symbol */
	sym_rv_park = 8215, /* RV park symbol */
	sym_school = 8216, /* school symbol */
	sym_stadium = 8217, /* stadium symbol */
	sym_store = 8218, /* dept. store symbol */
	sym_zoo = 8219, /* zoo symbol */
	sym_gas_plus = 8220, /* convenience store symbol */
	sym_faces = 8221, /* live theater symbol */
	sym_ramp_int = 8222, /* ramp intersection symbol */
	sym_st_int = 8223, /* street intersection symbol */
	sym_weigh_sttn = 8226, /* inspection/weigh station symbol */
	sym_toll_booth = 8227, /* toll booth symbol */
	sym_elev_pt = 8228, /* elevation point symbol */
	sym_ex_no_srvc = 8229, /* exit without services symbol */
	sym_geo_place_mm = 8230, /* Geographic place name, man-made */
	sym_geo_place_wtr = 8231, /* Geographic place name, water */
	sym_geo_place_lnd = 8232, /* Geographic place name, land */
	sym_bridge = 8233, /* bridge symbol */
	sym_building = 8234, /* building symbol */
	sym_cemetery = 8235, /* cemetery symbol */
	sym_church = 8236, /* church symbol */
	sym_civil = 8237, /* civil location symbol */
	sym_crossing = 8238, /* crossing symbol */
	sym_hist_town = 8239, /* historical town symbol */
	sym_levee = 8240, /* levee symbol */
	sym_military = 8241, /* military location symbol */
	sym_oil_field = 8242, /* oil field symbol */
	sym_tunnel = 8243, /* tunnel symbol */
	sym_beach = 8244, /* beach symbol */
	sym_forest = 8245, /* forest symbol */
	sym_summit = 8246, /* summit symbol */
	sym_lrg_ramp_int = 8247, /* large ramp intersection symbol */
	sym_lrg_ex_no_srvc = 8248, /* large exit without services smbl */
	sym_badge = 8249, /* police/official badge symbol */
	sym_cards = 8250, /* gambling/casino symbol */
	sym_snowski = 8251, /* snow skiing symbol */
	sym_iceskate = 8252, /* ice skating symbol */
	sym_wrecker = 8253, /* tow truck (wrecker) symbol */
	sym_border = 8254, /* border crossing (port of entry) */
	/*---------------------------------------------------------------
	Symbols for aviation (group 2...16383-24575...bits 15-13=010).
	---------------------------------------------------------------*/
	sym_airport = 16384, /* airport symbol */
	sym_int = 16385, /* intersection symbol */
	sym_ndb = 16386, /* non-directional beacon symbol */
	sym_vor = 16387, /* VHF omni-range symbol */
	sym_heliport = 16388, /* heliport symbol */
	sym_private = 16389, /* private field symbol */
	sym_soft_fld = 16390, /* soft field symbol */
	sym_tall_tower = 16391, /* tall tower symbol */
	sym_short_tower = 16392, /* short tower symbol */
	sym_glider = 16393, /* glider symbol */
	sym_ultralight = 16394, /* ultralight symbol */
	sym_parachute = 16395, /* parachute symbol */
	sym_vortac = 16396, /* VOR/TACAN symbol */
	sym_vordme = 16397, /* VOR-DME symbol */
	sym_faf = 16398, /* first approach fix */
	sym_lom = 16399, /* localizer outer marker */
	sym_map = 16400, /* missed approach point */
	sym_tacan = 16401, /* TACAN symbol */
	sym_seaplane = 16402, /* Seaplane Base */
};

std::string DescribeSymbolType(int smbl);

struct D108_Wpt_Type
{
	BYTE wpt_class;
	BYTE color;
	BYTE dspl;
	BYTE attr;
	int smbl;
	char subclass[18];
	Semicircle_Type posn;
	float alt;
	float dpth;
	float dist;
	char state[2];
	char cc[2];
	std::string ident;
	std::string comment;
	std::string facility;
	std::string city;
	std::string addr;
	std::string cross_road;
};

#endif /* GARMIN_PACKET_IDS_H */
