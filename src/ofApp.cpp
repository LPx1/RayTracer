#include "ofApp.h"

// Intersect Ray with Plane  (wrapper on glm::intersect*
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = normal;
	}
	return (hit);
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}



// This could be drawn a lot simpler but I wanted to use the getRay call
// to test it at the corners.
// 
void RenderCam::drawFrustum() {
	view.draw();
	Ray r1 = getRay(0, 0);
	Ray r2 = getRay(0, 1);
	Ray r3 = getRay(1, 1);
	Ray r4 = getRay(1, 0);

	/*	getRay(float u, float v) {
		glm::vec3 pointOnPlane = view.toWorld(u, v);
		return(Ray(position, glm::normalize(pointOnPlane - position)));*/

	float dist = glm::length((view.toWorld(0, 0) - position));
	r1.draw(dist);
	r2.draw(dist);
	r3.draw(dist);
	r4.draw(dist);

/* -----------------------------------------------------------*/

	float pixelWidth = (float)1 / 6; //pixel Width
	float pixelHeight = (float)1 / 4; // pixel Height
	float imageU = pixelWidth / 2;
	float imageV = pixelHeight / 2; // pixel Height

	Ray r5 = getRay(imageU, imageV);   //  getRay(.25, .25) /= getRay((1/4),(1/4))
	Ray r6 = getRay(imageU + (pixelWidth * 1) , imageV + (pixelHeight * 1));


	float dist1 = glm::length(position);

	/* --------------------------------------------------------------------------- */
	vector<Ray> rays;
	int count = 0;

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++)
		{
						//rays.push_back(getRay(imageU + (pixelWidth * i), imageV + (pixelHeight * j)));
						//rays[count].draw(20);
						//count++;
			Ray test = getRay(imageU + (pixelWidth * i), imageV + (pixelHeight * j));
			ofSetColor(ofColor::greenYellow);
			test.draw(20);
		}
	}

}

void ofApp::rayTrace() {

	float pixelWidth = (float)1 / imageWidth; //pixel Width in relation to UV
	float pixelHeight = (float)1 / imageHeight; // pixel Height
	float imageU = pixelWidth / 2;
	float imageV = pixelHeight / 2; // pixel Height
	phongPower = powSlider;

	ofImage img;
	vector<Ray> rays;
	int count = 0;
	glm::vec3 point, normal; // points of intersection
	img.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
	// **************************************
	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++)
		{
			Ray x = renderCam.getRay(imageU + (pixelWidth * i), imageV + (pixelHeight * j));
			int hold = -1;
			glm::vec3 holdP = glm::vec3(0, 0, 0);
			glm::vec3 holdN = glm::vec3(0, 0, 0);

			float closest = glm::length(INFINITY); //Grabs the closest intersection, sets to infinity to start
			for (int b = 0; b < scene.size(); b++)
			{
				// point of intersection - position of the camera = v 
				// (v being the distance between the point of intersection and the position of the camera
				bool intersect = scene[b]->intersect(x, point, normal);
				float  distance = glm::length (point - renderCam.position);
				
				
				if (intersect == true && distance < closest)
				{
					hold = b;
					holdP = point;
					holdN = normal;
					closest = distance;		

					//ofDrawSphere(point, 1); // For testing intersection on objects 
				}
			}
			if (hold > -1)
			{
				//img.setColor(i, imageHeight - j - 1, scene[hold]->diffuseColor);
				ofColor difCol = scene[hold]->diffuseColor;
				ofColor specCol = scene[hold]->specularColor;

				/*Calculates Lambert and Phong shading
					Sum of all lights is calculated inside each shading function*/

				ofColor colo = (difCol * 0.2) + lambert(holdP, holdN, difCol) + phong(holdP,holdN,difCol,specCol,phongPower);

				img.setColor(i, imageHeight - j - 1, colo);


			}
			else //If no intersection set the pixel color to black
			{
				img.setColor(i, imageHeight - j - 1, ofColor::black);
			}
		}
	}
	img.save("test.jpg");

}
// Calculate ambient shading 
//
ofColor ofApp::ambient(const ofColor ambient) {
	ofColor shading = (0,0,0);
	for (int i = 0; i < lights.size(); i++) {
		shading += ofFloatColor(ambient * lights[i]->intensity);
	}

	return shading;
}
// Calculates Lambert Shading
// p being the point of intersection, norm is the normal and
// diffuse is the diffuse color of the scene object.
//
ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse) {
	ofColor pixelColor = (0,0,0); //Sets to black
	ofColor difTotal;

	glm::vec3 l;
	glm::vec3 n;
	glm::vec3 poi, nor; //point and normal
	bool intersec;

	//Add up the sum of all light effects on pixel (Lambert)
	//
	for (int i = 0; i < lights.size() ;i++) {
		
		l = glm::normalize(lights[i]->position - p); // glm::normalize(lights[0]->position - p)
		n = norm; //Already normalized
		Ray shadow = Ray(p + glm::vec3(1,1,1) , l);

		float cos = fmax(0, glm::dot(l, n)); //  fmax(0, (float) glm::dot(l, n))
		float inten = (lights[i]->intensity / pow(glm::length(l), 2));
		pixelColor += ofFloatColor(diffuse * inten * cos);

		for (int b = 0; b < scene.size(); b++)
     {
      	intersec = scene[b]->intersect(shadow, poi, nor);
		
		//If there is no object in the way of the light there color that with diffuse
		//else if there is an object in the way color that black
		if (intersec == true) {
			return pixelColor = (10, 10, 10);
		}
      }
	}

	//Surface color * Light Intensity * dotProdut of Light and Normal

	/*------------------------------------------------------*/

	return pixelColor;
}

ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse,
	const ofColor specular, float power) {
	glm::vec3 l;
	glm::vec3 n;
	glm::vec3 v;
	glm::vec3 h;

	ofColor pixelColor = (0,0,0); //Sets color to black
	//Add up the sum of all light effects on pixel (Phong)
	//
	for (int i = 0; i < lights.size(); i++) {
		l = glm::normalize(lights[i]->position - p);
		n = norm; //Already normalized
		v = glm::normalize(renderCam.position - p);
		h = (v + l) / (glm::length(v + l)); //******

		float inten = (lights[i]->intensity / pow(glm::length(l), 2));
		float cosAlpha = fmax(0, glm::dot(n, h));
		cosAlpha = glm::pow(cosAlpha, power);
		// Specular Color * Light Intensity * (dotProduct of normal half vector)^power

		pixelColor += ofFloatColor(specular * inten * cosAlpha);
	}
	return pixelColor;
}


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetBackgroundColor(ofColor::black);
	mainCam.setDistance(30);
	mainCam.setNearClip(.1);
	sideCam.setPosition(40, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));
	theCam = &mainCam;
	previewCam.setPosition(renderCam.position);
	ofImage image;

	intensity1 = 1;
	intensity2 = 1;
	phongPower = 10;

	//Gui Setup
	gui.setup();
	gui.add(powSlider.setup("Phong Power Value",10, 1, 10000));
	gui.add(intenSlider1.setup("Light 1 insensity", .5, 0, 10));
	gui.add(intenSlider2.setup("Light 2 insensity", .5, 0, 10));

	//Create scene objects 
	scene.push_back(new Sphere (glm::vec3(2, -1, 0), 1.0, ofColor::gold));
	scene.push_back(new Sphere (glm::vec3(-1, 0, -5), 2.0, ofColor::darkBlue));
	scene.push_back(new Sphere(glm::vec3(4, 0, -4), 2.0, ofColor::crimson));
//	scene.push_back(new Sphere(glm::vec3(-1, -1, -1), 1.0, ofColor::darkBlue));
	scene.push_back(new Sphere(glm::vec3(-5, 0, -5), 2.0, ofColor::yellowGreen));
    scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));
	lights.push_back(new Light(glm::vec3(0, 4, 10), 0.1 , intensity1 , ofColor::white));
//	lights.push_back(new Light(glm::vec3(0, 5, 0), 0.1, intensity2, ofColor::white));
	lights.push_back(new Light(glm::vec3(8, 4, 10), 0.1, intensity2, ofColor::white));

}

//--------------------------------------------------------------
void ofApp::update() {
	lights[0]->intensity = intenSlider1;
	lights[1]->intensity = intenSlider2;
	phongPower = powSlider;
}

//--------------------------------------------------------------
void ofApp::draw() {

	gui.draw();
	theCam->begin();

	ofNoFill();
	for (int i = 0; i < scene.size(); i++) {
		ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();
	}

	for (int i = 0; i < lights.size(); i++) {
		ofSetColor(lights[i]->diffuseColor);
		lights[i]->draw();
	}

	Ray shadow = Ray(glm::vec3(4, 0, -4), (glm::vec3(10,2,5)-glm::vec3(4,0,-4)));
	
	shadow.draw(1); //Testing ray for shadows

	ofSetColor(ofColor::lightSkyBlue);
	renderCam.drawFrustum();

	ofSetColor(ofColor::blue);
	renderCam.draw();

//	rayTrace(); //for testing

	theCam->end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'F':
	case 'b':
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case 'h':
		bHide = !bHide;
		break;
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3: //Switches to previewing what renderCam can see
		theCam = &previewCam;
		break;
	case ' ':
		rayTrace();
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

