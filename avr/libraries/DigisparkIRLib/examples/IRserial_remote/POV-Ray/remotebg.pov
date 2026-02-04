//POV-Ray source to generate the background image for IRserial_remote
// create rectangular areas with rounded corners for use as 
// buttons and background objects.

// Render at 1024x768 then crop 312 pixels from each side
// leaving 400x768 final image.
#declare Area=15;  //size of area lights
#declare CR=0.1;   //corner radius                 
#declare ER= 0.5;  //edge radius
#declare CX= 3;    //width from corner to corner                  
#declare CY= 7.75; //height from corner to corner                             
#declare BZ=-ER;   //Z offset for buttons

plane {z,0 pigment{rgb<0.8,0.85,1>*0.8}}//background

#macro Thing (ER,CR,CX,CY,T)
  #local Corner=                         
    union {
      torus {CR,ER rotate x*90}
      cylinder {ER*z,-ER*z,CR}
    }
  union {
    object{Corner translate< CX,CY,0>}
    object{Corner translate<-CX,CY,0>}
    object{Corner translate< CX,-CY,0>}
    object{Corner translate<-CX,-CY,0>}
    cylinder{CY*y,-CY*y,ER translate<-CX-CR,0,0>}
    cylinder{CY*y,-CY*y,ER translate< CX+CR,0,0>}
    cylinder{CX*x,-CX*x,ER translate<0,-CY-CR,0>}
    cylinder{CX*x,-CX*x,ER translate<0, CY+CR,0>}
    box{<-CX,-CY-CR,-ER><CX,CY+CR,ER>}
    box{<-CX-CR,-CY,-ER><CX+CR,CY,ER>}
    texture {T}
 }    
#end

#declare BX= 0.4; #declare BY=BX;//size of the buttons
#declare White_Texture=texture{pigment{rgb 1}finish {ambient 0.3}}
#declare Blue_Texture=texture{pigment {rgb<0.85,0.9 ,1>}}

object {Thing(ER,CR,CX,CY, White_Texture)}//main object
//loop through the buttons
#declare R=-4.5;
#while (R<5.5) 
   #declare C=-1.5;
   #while (C<=1.5)
      object{Thing(0.1,0.2,(BX*0.8),(BY*0.8), Blue_Texture)
        translate <C*BX*4,R*BY*4,BZ>
      }
      #declare C=C+1;
   #end
   #declare R=R+1;
#end


light_source{<50,50,-100>*5 color 0.8 
    #if (Area)area_light x*Area,y*Area,9,9#end
}
light_source{<0,0,-400>*3 rgb 1}

camera{orthographic location <0,0,-120> look_at <0,0,0>  angle 11 } 
//That's all folks!