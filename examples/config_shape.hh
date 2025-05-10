#include <string>
#include <vector>

#include "../physon.hh"
#include "../physon_types.hh"

#include "shape.hh"
#include "config.hh"


class ConfigShape : Config {

    std::vector<Shape> shapes;
    /** Returns the float 2d-points of a json-point array, e.g. [[0.0, 0.0],[1.0, 1.0]] */
    std::vector<Point> unwrap_point_array(json_array_wrap point_array);

public:

    ConfigShape(std::string config_string) :  Config(config_string) {};
    std::vector<Shape>& load_shapes();
};




std::vector<Point> ConfigShape::unwrap_point_array(json_array_wrap point_array){
    std::vector<Point> points;

    for(int i = 0; i < point_array.size(); i++){

        JsonWrapper point_wrapper = point_array[i];

        // Unwrap point
        json_array_wrap point_array = physon.unwrap_array(point_wrapper);

        json_float point_x = physon.unwrap_float(point_array[0]);
        json_float point_y = physon.unwrap_float(point_array[1]);

        Point point = {point_x, point_y};
        points.push_back(point);

    }

    return points;
}



std::vector<Shape>& ConfigShape::load_shapes(){

    physon.parse();
    
    JsonWrapper root_object_wrapper = physon.root_wrapper;
    json_object_wrap root_object = physon.unwrap_object(root_object_wrapper);

    // Loop shapes
    for(JsonWrapper kv_wrap : root_object){
        json_kv_wrap kv = physon.unwrap_kv(kv_wrap);

        Shape new_shape;

        std::string shape_name = kv.first;
        json_array_wrap point_array = physon.unwrap_array(kv.second);


        if(shape_name == "line"){
            new_shape.type = SHAPE::LINE;

            // JsonWrapper point_1_wrapper = point_array[0];
            // JsonWrapper point_2_wrapper = point_array[1];

            // // Unwrap point 1
            // json_array_wrap point_1_array = physon.unwrap_array(point_1_wrapper);
            // json_float point_1_x = physon.unwrap_float(point_1_array[0]);
            // json_float point_1_y = physon.unwrap_float(point_1_array[1]);
            // Point point_1 = {point_1_x, point_1_y};
            // new_shape.points.push_back(point_1);
            
            // // Unwrap point 2
            // json_array_wrap point_2_array = physon.unwrap_array(point_2_wrapper);
            // json_float point_2_x = physon.unwrap_float(point_2_array[0]);
            // json_float point_2_y = physon.unwrap_float(point_2_array[1]);
            // Point point_2 = {point_2_x, point_2_y};
            // new_shape.points.push_back(point_2);
        }


        new_shape.points = unwrap_point_array(point_array);

        shapes.push_back(new_shape);

    }

    return shapes;
}
