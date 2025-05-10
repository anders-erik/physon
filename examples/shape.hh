#include <string>
#include <vector>

#include "../physon.hh"
#include "../physon_types.hh"



enum class SHAPE {
    LINE,
};

struct Point { double x; double y; };

struct Shape {
    SHAPE type;
    std::vector<Point> points;

    void print();
};

void Shape::print(){
    std::cout << std::endl;
        std::cout << "Shape.print() : " << std::endl;
        

        if(type == SHAPE::LINE)
            std::cout << "LINE" << std::endl;
        
        for(Point& point : points){
            std::cout << "    x :" << point.x << ", y : " << point.y << std::endl;
            
        }

        std::cout << std::endl;
}


