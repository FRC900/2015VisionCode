#include <vector>
#include <utility>

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"

namespace bg = boost::geometry;

using namespace bg;
using namespace std;

typedef bg::model::d2::point_xy<float> Point;
typedef bg::model::polygon<Point> Polygon;
typedef bg::model::segment<Point> Segment;


class Robot {
private:
	float fov;
	Point loc;
	float rot; //rot means rotation from now on. Rotation is always going to be where x axis is 0 degrees.

	float toHorizontalFOV(float diag, int width, int height) {
		return cos(atan(height / width)) * diag; //takes a diagonal field of view (given as specs for most cameras) and converts to horizontal (useful)
		//assumes that the image is not being streteched to the current size
	}

public:

	Robot(float loc_x, float loc_y, float _rot, float _fov, bool horizontalFOV=false, int imageWidth=640, int imageHeight=480) {
		Point _loc(loc_x,loc_y);
		if(horizontalFOV)
			fov = _fov;
		else
			fov = toHorizontalFOV(_fov,imageWidth,imageHeight);
		rot = _rot;
	}

	adjust(float deltaX, float deltaY, float deltaRot) { //updates robot position, meant to be called every frame
		Point deltaPoint(deltaX,deltaY); 
		bg::add_point(loc,deltaPoint);
		rot = rot + deltaRot;
	}

	Point getLoc() {
		return loc;
	}

	float getRotation() {
		return rot;
	}

	std::pair<float,float> getBoundingAngles() { //this returns the two angles that define the fov cone. Don't forget that we are in 2D so fov is a triangle
		std::pair<float,float> return_pair; //this function is useful for calculating the fov polygon
		return_pair.first = rot - 0.5 * fov;
		return_pair.second = rot + 0.5 * fov;
		return return_pair;
	}

};

class Field {
private:
	Polygon data;
public:
	Field(vector<Point> _data) {
		for(int i = 0; i < _data.size(); i++) {
			data.outer().push_back(_data[i]);
		}
		data.outer().push_back(_data[0]); //must be a closed polygon
	}

	Polygon getAsPoly() {
		return data;
	}

	vector<Segment> getAsSegments() {
		vector<Segment> segment_list;
		vector<Point> data_point_list = data.outer();
		Segment current_segment;
		for(int i = 0; i < data_point_list.size(); i++) {
			if(i == data_point_list.size() - 1) //checks if this is the last point
				bg::assign_values(current_segment, bg::get<0>(data_point_list[i]), bg::get<1>(data_point_list[i]), bg::get<0>(data_point_list[0]), bg::get<1>(data_point_list[0]) );
			else
				bg::assign_values(current_segment, bg::get<0>(data_point_list[i]), bg::get<1>(data_point_list[i]), bg::get<0>(data_point_list[i+1]), bg::get<1>(data_point_list[i+1]) );
			segment_list.push_back(current_segment);
		}
		return segment_list;
	}

	vector<Point> getAsPoints() {
		return data.outer();
	}



};

class Fvs {
private:

	Polygon fovPoly(Field field, Robot bot) {
		Polygon fieldPoly = field.getAsPoly();
		if(!bg::within(bot.getLoc(), fieldPoly)) { //must be in the field!
			Polygon empty;
			return empty;
		}
		std::pair<float,float> boundingAngles = bot.getBoundingAngles();
		Point projectionP1(1000 * cos(boundingAngles.first),1000 * sin(boundingAngles.first) ); //this is a point really far away so that we 
		Point projectionP2(1000 * cos(boundingAngles.second),1000 * sin(boundingAngles.second) ); //can do intersection stuff

		Polygon projection_poly;
		projection_poly.outer().push_back(bot.getLoc());
		projection_poly.outer().push_back(projectionP1);
		projection_poly.outer().push_back(projectionP2);

		std::deque<Polygon> fovPoly_list;
		bg::intersection(projection_poly,fieldPoly,fovPoly_list);

		return fovPoly_list[0];
	}
	
public:

};


int main() {
	vector<Point> first_polygon;

	Point p1(0,0);
	Point p2(0,1);
	Point p3(1,1);
	Point p4(1,0);

	first_polygon.push_back(p1);
	first_polygon.push_back(p2);
	first_polygon.push_back(p3);
	first_polygon.push_back(p4);

	Field square_field(first_polygon);


	return 0;
}