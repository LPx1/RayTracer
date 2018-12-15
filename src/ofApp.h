//
//  RayCaster - Set of simple classes to create a camera/view setup for our Ray Tracer HW Project
//
//  I've included these classes as a mini-framework for our introductory ray tracer.
//  You are free to modify/change.   
//
//  These classes provide a simple render camera which can can return a ray starting from
//  it's position to a (u, v) coordinate on the view plane.
//
//  The view plane is where we can locate our photorealistic image we are rendering.
//  The field-of-view of the camera by moving it closer/further 
//  from the view plane.  The viewplane can be also resized.  When ray tracing an image, the aspect
//  ratio of the view plane should the be same as your image. So for example, the current view plane
//  default size is ( 6.0 width by 4.0 height ).   A 1200x800 pixel image would have the same
//  aspect ratio.
//
//  This is not a complete ray tracer - just a set of skelton classes to start.  The current
//  base scene object only stores a value for the diffuse/specular color of the object (defaut is gray).
//  at some point, we will want to replace this with a Material class that contains these (and other 
//  parameters)
//  
//  (c) Kevin M. Smith  - 24 September 2018
//
#pragma once

#include "ofMain.h"
#include "ofxGui.h"

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }

	// commonly used transformations
	//
	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}
	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}
	glm::mat4 getScaleMatrix() {
		return (glm::scale(glm::mat4(1.0), glm::vec3(scale.x, scale.y, scale.z)));
	}


	glm::mat4 getLocalMatrix() {

		// get the local transformations + pivot
		//
		glm::mat4 scale = getScaleMatrix();
		glm::mat4 rotate = getRotateMatrix();
		glm::mat4 trans = getTranslateMatrix();

		// handle pivot point  (rotate around a point that is not the object's center)
		//
		glm::mat4 pre = glm::translate(glm::mat4(1.0), glm::vec3(-pivot.x, -pivot.y, -pivot.z));
		glm::mat4 post = glm::translate(glm::mat4(1.0), glm::vec3(pivot.x, pivot.y, pivot.z));



		return (trans * post * rotate * pre * scale);

	}

	glm::mat4 getMatrix() {

		// if we have a parent (we are not the root),
		// concatenate parent's transform (this is recursive)
		// 
		if (parent) {
			glm::mat4 M = parent->getMatrix();
			return (M * getLocalMatrix());
		}
		else return getLocalMatrix();  // priority order is SRT
	}

	// get current Position in World Space
	//
	glm::vec3 getPosition() {
		return (getMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
	}

	// set position (pos is in world space)
	//
	void setPosition(glm::vec3 pos) {
		position = glm::inverse(getMatrix()) * glm::vec4(pos, 1.0);
	}

	// return a rotation  matrix that rotates one vector to another
	//
	glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2)
	{

		glm::vec3 axis = glm::cross(v1, v2);
		glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
		return glm::toMat4(q);
	}

	//  Hierarchy 
	//
	void addChild(SceneObject *child) {
		childList.push_back(child);
		child->parent = this;
	}

	SceneObject *parent = NULL;        // if parent = NULL, then this obj is the ROOT
	vector<SceneObject *> childList;

	// position/orientation 
	//
	glm::vec3 position = glm::vec3(0, 0, 0);   // translate
	glm::vec3 rotation = glm::vec3(0, 0, 0);   // rotate
	glm::vec3 scale = glm::vec3(1, 1, 1);      // scale

	// rotate pivot
	//
	glm::vec3 pivot = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	// UI parameters
	//
	bool isSelectable = true;
	string name = "SceneObject";
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofDrawSphere(position, radius);
	}

	float radius = 1.0;
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};

// General Light Source
//
class Light : public SceneObject {
public:
	Light(glm::vec3 p, float r, float inten, ofColor diffuse = ofColor::white) { position = p; radius = r; diffuseColor = diffuse; intensity = inten; }
	Light() {}
	 
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofDrawSphere(position, radius);
	}
	float intensity = 1;
	float radius = 0.1;
//	ofColor diffuseColor;
};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	// Changed natural color of Plane (diffuse)
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::whiteSmoke, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		isSelectable = false;
		plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() {
		plane.rotateDeg(-90, 1, 0, 0);
		isSelectable = false;
	}

	//bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
	//	return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	//}

	glm::vec3 normal = glm::vec3(0, 1, 0);
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
//	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofPlanePrimitive plane;
	float width = 20;
	float height = 20;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
		for (int i = 0; i < 6; i++) {
			for (int j = 3; j >= 0; j--)
			{
				ofDrawRectangle(glm::vec3(min.x + i, min.y + j, position.z), 1, 1);
			}
		}
//		ofDrawGridPlane();
	}


	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};



class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	void drawGrid();
	void drawAxis(glm::vec3 position);
	bool objSelected() { return (selected.size() ? true : false); };
	bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point);
//	float dotProduct(glm::vec3 lighting, glm::vec3 norm);

	// Formulas function to help generate shading
	// Functions will return the a color after the color is calculated from the light
	ofColor lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse);
	ofColor phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse,
		const ofColor specular, float power);
	ofColor ambient(const ofColor ambient);



//	bool bHide = true;
	bool bShowImage = false;

	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera  *theCam;    // set to current camera either mainCam or sideCam
	
	ofxPanel gui;
	ofxFloatSlider powSlider;
	ofxFloatSlider intenSlider1;
	ofxFloatSlider intenSlider2;

	// set up one render camera to render image throughn
	//
	RenderCam renderCam;
	ofImage image;

	vector<SceneObject *> scene;
	vector<SceneObject *> selected;
	vector<Light *> lights;

	// state
	bool bDrag = false;
	bool bHide = true;
	bool bAltKeyDown = false;
	bool bRotateX = false;
	bool bRotateY = false;
	bool bRotateZ = false;
	bool child = false;
	bool rotation = false;
	bool playback = false;
	glm::vec3 lastPoint;


	int imageWidth = 1024; //Original 600,400
	int imageHeight = 768;
	float phongPower;
	float intensity1;
	float intensity2;
//	int imageWidth = 6;
//	int imageHeight = 4;

};
