#!/usr/bin/env python
# coding=utf-8

import roslib
roslib.load_manifest('sandoval_penilla')
import rospy
import math
import tf
import sys
from geometry_msgs.msg import Twist

NOMBRE = "sandoval_penilla"

class Practica02:
    """
    ==============================================
    Practica_02 Sandoval Penilla Oscar
    UNAM Facultad de Ingenieria
    Temas Selectos de MEcatronica
    
       
    ==============================================
    """
    
    def __init__(self):
        print("PRÀCTICA 02 - Control de Posición - {}".format(NOMBRE))
        self.n = rospy.init_node("practica_02")
        self.run = False
        
        # Obtencion de parametros
        self.a = 0.0 # 0.0 rad por defecto
        self.type = "diff" #metodo diff holonomico por defecto
        
        
        
        # validacion de parametros        
        if rospy.has_param("~x") and rospy.has_param("~y"):
            self.xg = rospy.get_param("~x")
            self.yg = rospy.get_param("~y")      
            self.run = True
        else: print("parmeters X and Y are mandatory")
           
        if rospy.has_param("~type"):
            if rospy.get_param("~type") == "diff" or rospy.get_param("~type") == "omni":
                self.type = rospy.get_param("~type")
            else:
                print("Valid control types are \omni\ and \diff\ ")
                self.run = False
        
        if rospy.has_param("~a"):
            self.a = rospy.get_param("~a")
            

        # Borramos los parametros para una nueva ejecucion
        try:
            rospy.delete_param("~x")
            rospy.delete_param("~y")
            rospy.delete_param("~a")
            rospy.delete_param("~type")
        except KeyError:
            pass
            
        
        # Creamos nuestro mensaje de tipo twist        
        self.m = Twist()
        # Creamos nuestro listener de transformaciones
        self.listener = tf.TransformListener()
        # Creamos un publisher para publicar el mensaje twist
        self.publisher = rospy.Publisher('/hardware/mobile_base/cmd_vel',
                                  Twist,
                                  queue_size=1)
                                  
                                  
    def main(self):
        if self.run and self.type == "diff":
            print("metodo diff")
            self.holonomico()
        elif self.run and self.type == "omni":
            print("metodo omni")
            self.omni()
        
        
            
            
        
        
        
    def omni(self):
        print("Executing position control for an OMNIDIRECTIONAL base with \n Goal X={} \tGoal Y={} \tGoal Angle={}".format(self.xg,self.yg,self.a))            
        rate = rospy.Rate(10)
        while not rospy.is_shutdown():
            try:
                self.listener.waitForTransform("map", "base_link", 
                                          rospy.Time(), 
                                          rospy.Duration(1.0))
                (trans,rot) = self.listener.lookupTransform('map',
                                                            'base_link',
                                                             rospy.Time(0))
                tf = self.listener.lookupTransform('map',
                                                            'base_link',
                                                             rospy.Time(0))
                
            except (tf.LookupException, tf.ConnectivityException, 
                    tf.ExtrapolationException):
                    pass
                
                            
            xr = trans[0]
            yr = trans[1]
            v_max = 0.5
            a = 1
            thr = math.atan2(rot[2],rot[3])*2
            eth = math.atan2(self.yg - yr, self.xg - xr) - thr
            #distancia al punto
            dist = math.sqrt((self.yg - yr)**2 + (self.xg - xr)**2)
            if eth < -math.pi:
                eth += 2*math.pi
            elif eth > math.pi:
                eth += -2*math.pi
                
            
            
            b = 1
            w_max = 1   
            v = v_max*math.exp((-eth**2)/a)
            w = w_max*(((2)/(1+math.exp(-eth/b)))-1)
            
            self.m.linear.x = v
            if dist < 0.1:
                self.m.linear.x = 0
            self.m.angular.z = w
#            if math.fabs(eth) < 0.1:
#                self.m.angular.z = 0
            self.publisher.publish(self.m)
            
            
            
            #print("""
#error   = {}            
#angle   = {}º
#dist    = {}
#linear  = {}
#angular = {}
#   x    = {}
#   y    = {}""".format(eth,math.degrees(thr),dist,v,w,xr,yr))
            
            
            rate.sleep()

    def holonomico(self):
        print("Executing position control for an DIFFERENTIAL base with \n Goal X={} \tGoal Y={} \tGoal Angle={}".format(self.xg,self.yg,self.a))            
        rate = rospy.Rate(10)
        while not rospy.is_shutdown():
            try:
                self.listener.waitForTransform("map", "base_link", 
                                          rospy.Time(), 
                                          rospy.Duration(1.0))
                (trans, rot) = self.listener.lookupTransform('map',
                                                            'base_link',
                                                             rospy.Time(0))
            except (tf.LookupException, tf.ConnectivityException, 
                    tf.ExtrapolationException):
                pass
            
            kp = 1            
            xr = trans[0]
            yr = trans[1]
            
            
            ex = self.xg - xr
            ey = self.yg - yr
            
            vx = kp*ex
            vy = kp*ey
            
            #print("error = {}".format(xr))
            
            self.m.linear.x = vx;
            self.m.linear.y = vy;
            self.publisher.publish(self.m)
            
            rate.sleep()


if __name__ == '__main__':
    
    try:
        prac = Practica02()
        prac.main()
        
    except rospy.ROSInterruptException:
        pass

