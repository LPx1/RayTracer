# RayTracing
Created a rendering camera and a viewing environment for objects such as a spheres and a plane. Implemented a ray tracing algorithm to trace with depth ordering.  

 Lambert and Blinn-Phong shading are implemented with multiple lights in addition to having shadow rendering implemented as well. 

RayTrace result: 

![](https://github.com/LPx1/RayTracing/blob/master/bin/data/RT_Result.jpg)

 
EDIT 12/18/18 :

Ambient Coloring added to intersected objects that to account for not fully black shading when not in direct contact of lights.

Anti Aliasing applied, with 16 rays casted for each pixel at random locations within the pixel. To account for more angular objects with a more complicated structure. 

Raytracing algorithm updated to account for recursive ray tracing methods, such as reflection. Reflection is account by casting reflected rays from the intersected pointed to other locations and account the intersected object's color and add it into our object to account for color.  

https://www.behance.net/gallery/72712785/Raytracing
