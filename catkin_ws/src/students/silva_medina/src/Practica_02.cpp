/*
 * ROBOTS MÓVILES Y AGENTES INTELIGENTES
 * FACULTAD DE INGENIERÍA, UNAM, 2019-1
 * P R Á C T I C A   2
 * CONTROL DE POSICIÓN
 */

#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "tf/transform_listener.h"
#include <math.h>
#define NOMBRE "silva_medina"
#define PI 3.1415926535897
#define vmax 0.5

int main(int argc, char** argv)
{
    std::cout << "PRÁCTICA 02 - Control de Posición - " << NOMBRE << std::endl;
    ros::init(argc, argv, "practica_02");
    ros::NodeHandle n("~");

    if(!n.hasParam("x") || !n.hasParam("y"))
    {
	std::cout << "Parameters \"x\" and \"y\" are required. " << std::endl;
	return -1;
    }
    //Variables required for the position control
    float robot_x;
    float robot_y;
    float robot_a;
    float goal_x;
    float goal_y;
    float goal_a;
    std::string control_type = "";
	
    //Variables utilizadas para el cálculo de errores

    float error_x,error_y,error_a;

    //Declaracion de constantes en la formula

    float k = 0.5, alpha = 0.5, betha = 1.0;

    //
	
    //Getting parameteres to set goal position and control type.
    n.param<std::string>("type", control_type, "diff"); //The default control type is for a differential base
    n.param<float>("a", goal_a, 0.0);                   //The default goal angle is 0.0 rad
    if(!n.getParam("x", goal_x))
    {
	std::cout << "Invalid x value." << std::endl;
	return -1;
    }
    if(!n.getParam("y", goal_y))
    {
	std::cout << "Invalid y value." << std::endl;
	return -1;
    }
    if(control_type != "diff"  && control_type != "omni")
    {
	std::cout << "Valid control types are \"omni\" and \"diff\"." << std::endl;
	return -1;
    }
    
    if(control_type == "omni")
	std::cout << "Executing position control for an OMNIDIRECTIONAL base with " << std::endl;
    else
	std::cout << "Executing position control for a DIFFERENTIAL base with " << std::endl;
    std::cout << "Goal X=" << goal_x << "\tGoal Y=" << goal_y << "\tGoal Angle=" << goal_a << std::endl;

    //Required subscriptors, publishers and messages
    ros::Publisher pub_cmd_vel = n.advertise<geometry_msgs::Twist>("/hardware/mobile_base/cmd_vel", 1);
    ros::Rate loop(10);
    tf::TransformListener tl;
    tf::StampedTransform t;
    tf::Quaternion q;
    geometry_msgs::Twist msg_cmd_vel;

    while(ros::ok())
    {
	//Instructions needed to get the current robot position
	tl.waitForTransform("map", "base_link", ros::Time::now(), ros::Duration(0.5));
	tl.lookupTransform("map", "base_link", ros::Time(0), t);
	robot_x = t.getOrigin().x();
	robot_y = t.getOrigin().y();
	q = t.getRotation();
	robot_a = atan2(q.z(), q.w())*2;


		
	/*
	 * TODO:
	 *
	 * Calculate the values of msg_cmd_vel according to the control type.
	 * (for omnidirectional or differential base)
	 * Declare more variables if necessary.
	 * The maximum linear speed must be 0.5 m/s
	 * The maximul angular speed must be 0.5 rad/s
	 * The robot must stop if it is at 0.1 m or less from the goal point
	 */
	
	//Se calculan los errores en la velocidad en x,y y z	
		error_x = goal_x - robot_x;
		error_y = goal_y - robot_y;	

	//Se calcula el error de ángulo para dejarlo en el rango de PI a -PI
		if (control_type=="omni")
	   		error_a = goal_a - robot_a;
		else
			error_a = atan2(error_y,error_x) - robot_a;
	
		if(error_a < -PI)
        		error_a += 2*PI;
    		if (error_a > PI)
        		error_a -= 2*PI;


	if(fabs(error_x>=0.1) || fabs(error_y>=0.1)){//Mientras que el error en x ó en y sea mayor a 0.1 m

	
	
		if(control_type == "omni"){//Si el control es de tipo omnidireccional
 			
			//Formulas para calcular la velocidad lineal
        		msg_cmd_vel.linear.x = k * error_x * cos(robot_a) + k * error_y * sin(robot_a);
        		msg_cmd_vel.linear.y = -k * error_x * sin(robot_a) + k * error_y * cos(robot_a);

			//Si las velocidades lineal es mayor a 0.5, se modifican
			
			if (msg_cmd_vel.linear.x > 0.5)
		        	msg_cmd_vel.linear.x = vmax;
		        if (msg_cmd_vel.linear.y > 0.5)
            			msg_cmd_vel.linear.y = vmax;
        		if (msg_cmd_vel.linear.x < 0.5)
            		 	msg_cmd_vel.linear.x = -vmax;
        		if (msg_cmd_vel.linear.y < 0.5)
            			msg_cmd_vel.linear.y = -vmax;

			
		}        
		else{//Si el control es de tipo diferencial

		
			//Formulas para calcular la velocidad lineal en x y el angulo necesario para moverse
		
			msg_cmd_vel.linear.x = vmax * exp(-pow(error_a, 2) / alpha);
			msg_cmd_vel.angular.z = vmax * (2 / (1 + exp(-(error_a / betha))) - 1);	
			
		}


	}
	else{
		
		msg_cmd_vel.linear.x =0;
		msg_cmd_vel.linear.y =0;
		if(error_a!=0){// Se modifica el ángulo para obtener el deseado
							
			msg_cmd_vel.angular.z = 0.5*(goal_a - robot_a);			

					
		}
		else{
		msg_cmd_vel.angular.z =0;		
		}
	}	

	pub_cmd_vel.publish(msg_cmd_vel);
	ros::spinOnce();
	loop.sleep();
    }
    
    return 0;
    
}
