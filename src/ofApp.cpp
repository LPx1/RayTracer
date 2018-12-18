#include "ofApp.h"


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
	gui.add(refSlider.setup("Reflectivity", .8, 0, 1));
	gui.add(intenSlider1.setup("Light 1 insensity", .5, 0, 1));
	gui.add(intenSlider2.setup("Light 2 insensity", .5, 0, 1));

	//Create scene objects 
	scene.push_back(new Sphere (glm::vec3(2, -1, 0), 1.0, ofColor::gold));
	scene.push_back(new Sphere (glm::vec3(-1, 0, -5), 2.0, ofColor::darkBlue));
	scene.push_back(new Sphere(glm::vec3(4, 0, -4), 2.0, ofColor::crimson));
//	scene.push_back(new Sphere(glm::vec3(-1, -1, -1), 1.0, ofColor::darkBlue));
	scene.push_back(new Sphere(glm::vec3(-5, 0, -5), 2.0, ofColor::yellowGreen));
    scene.push_back(new Plane(glm::vec3(0, -2, -3), glm::vec3(0, 1, 0),ofColor::lightSlateGray));
	scene.push_back(new Plane(glm::vec3(0, 8, -3), glm::vec3(0, 1, 0), ofColor::lightSlateGray));


	lights.push_back(new Light(glm::vec3(0, 4, 10), 0.1 , intensity1 , ofColor::white));
//	lights.push_back(new Light(glm::vec3(0, 5, 0), 0.1, intensity2, ofColor::white));
	lights.push_back(new Light(glm::vec3(8, 4, 10), 0.1, intensity2, ofColor::white));

	float dd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - .5;
	printf("%lf", dd);
}

//--------------------------------------------------------------
void ofApp::update() {
	lights[0]->intensity = intenSlider1;
	lights[1]->intensity = intenSlider2;
	reflect = refSlider;
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

	// Draws parent object and highlights them when selected
//
	for (int i = 0; i < scene.size(); i++) {
		if (objSelected() && scene[i] == selected[0])
			ofSetColor(ofColor::gold);
		else ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();

		// Highlights child object selected
		//
		for (int j = 0; j < scene[i]->childList.size(); j++)
		{
			if (objSelected() && scene[i]->childList[j] == selected[0])
				ofSetColor(ofColor::gold);
			else ofSetColor(scene[i]->childList[j]->diffuseColor);
			//scene[i]->childList[j]->draw();

		}
	}

//	rayTrace(); //for testing

	theCam->end();
}


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
	Ray r6 = getRay(imageU + (pixelWidth * 1), imageV + (pixelHeight * 1));


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
			int n = 3;

			for (int p = 0; p < n - 1; p++)
			{
				for (int q = 0; q < n - 1; q++)
				{
					float u = imageU + (pixelWidth + p)  / n;
					float v = imageV + (pixelHeight + q) / n;
					Ray test = getRay(u,v);
					ofSetColor(ofColor::greenYellow);
					test.draw(20);
				}
			}


		}
	}

}

void ofApp::rayTrace() {

	phongPower = powSlider;
	int count = 0;
	int n = 4; //Number of rays to draw in pixel n^2

	img.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
	// **************************************

	float r, g, b;
	r = g = b = 0;

	srand(time(NULL));

	//Run through each pixel at (i,j)

	for (float i = 0; i < imageWidth; i++) {
		for (float j = 0; j < imageHeight; j++)
		{
			ofColor rayColor = (0, 0, 0);
			r = 0;
			g = 0;
			b = 0;

			for (float p = 0; p < n ; p++)
			{
				for (float q = 0; q < n ; q++)
				{
					float AA = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) -.5;
					float A2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) -.5;

					//printf("%lf", AA);
					//printf("%lf", A2);

					//float u = i + (pixelWidth + p) / n;	imageU + (pixelWidth * i )
					//float v = j + (pixelHeight + q) / n;	imageV + (pixelHeight * j )
					float u = (i + AA) / float(imageWidth);
					float v = (j + A2) / float(imageHeight);
					Ray x = renderCam.getRay(u, v);

					rayColor = trace(x, 0);

					
					r += rayColor.r;
					g += rayColor.g;
					b += rayColor.b;


					//Negate all the addition of shading 
					r /= 250;
					g /= 250;
					b /= 250;
				}
				
			}



			//rayColor = rayColor / 9;
			//img.setColor(i, imageHeight - j - 1, rayColor);

			rayColor = ofFloatColor(r, g, b);

			img.setColor(i, imageHeight - j - 1, rayColor);
		}
	}
	img.save("test.jpg");

}

ofColor ofApp::trace(const Ray &x, int depth) {
	int hold = -1;
	glm::vec3 holdP = glm::vec3(0, 0, 0);
	glm::vec3 holdN = glm::vec3(0, 0, 0);

	float closest = glm::length(INFINITY); //Grabs the closest intersection, sets to infinity to start
	//Loops through all objects in scene
	for (int b = 0; b < scene.size(); b++)
	{
		// point of intersection - position of the camera = v 
		// (v being the distance between the point of intersection and the position of the camera
		bool intersect = scene[b]->intersect(x, point, normal);
		float  distance = glm::length(point - renderCam.position);

		//Checks for the closest object
		if (intersect == true && distance < closest)
		{
			hold = b;
			holdP = point;
			holdN = normal;
			closest = distance;

			//ofDrawSphere(point, 1); // For testing intersection on objects 
		}
	}

	
	//If ray intersects an object then color it in with the closest objects color otherwise
	if (hold > -1)
	{
		//img.setColor(i, imageHeight - j - 1, scene[hold]->diffuseColor);
		ofColor difCol = scene[hold]->diffuseColor;
		ofColor specCol = scene[hold]->specularColor;

		/*Calculates Lambert and Phong shading
		Sum of all lights is calculated inside each shading function*/

		ofColor colo = (difCol * 0.2);

		colo += lambert(holdP, holdN, difCol);

		colo += phong(holdP, holdN, difCol, specCol, phongPower);


		if (depth < MAX_RAY_DEPTH) {
			ofVec3f reflectP = reflection(x.d, holdN);
			Ray reflectionRay = Ray(holdP + reflectP * 0.01 , reflectP);
			colo += reflect * trace(reflectionRay, depth + 1);
		}


		return colo;

		//	img.setColor(i, imageHeight - j - 1, colo);

	}
	else //If no intersection set the pixel color to black
	{
		//img.setColor(i, imageHeight - j - 1, COLOR_BACKGROUND);
		return COLOR_BACKGROUND;
	}
	

	//return ambient(scene[hold]->diffuseColor);
		
}

//Calculates the reflection off a surface 
ofVec3f ofApp::reflection(const glm::vec3 &dir, const glm::vec3 &norm)
{
	return dir - 2 *  norm * glm::dot(dir, norm);
}

// Calculate ambient shading 
//
ofColor ofApp::ambient(const ofColor ambient) {
	ofColor shading = (0, 0, 0);
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
	ofColor pixelColor = (0, 0, 0); //Sets to black
	ofColor difTotal;

	glm::vec3 l;
	glm::vec3 n;
	glm::vec3 poi, nor; //point and normal
	bool intersec;

	//Add up the sum of all light effects on pixel (Lambert)
	//
	for (int i = 0; i < lights.size(); i++) {

		l = glm::normalize(lights[i]->position - p); // glm::normalize(lights[0]->position - p)
		n = norm; //Already normalized
		Ray shadow = Ray(p + glm::vec3(1, 1, 1) * 0.01, l);

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

	ofColor pixelColor = (0, 0, 0); //Sets color to black
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
void ofApp::keyPressed(int key) {
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
	case 'R':
	case 'r':
		rayTrace();
		break;
	case ' ': //Dynamically create new objects for rayTracing
	{		ofColor colors[10] = { ofColor::blue, ofColor::red, ofColor::green,
								ofColor::purple, ofColor::gold, ofColor::magenta, 
	ofColor::teal, ofColor::blueSteel, ofColor::orangeRed, ofColor::oliveDrab };

	srand(time(NULL));
	int random = (rand() % 10); //Will select a random color from list of colors 


	scene.push_back(new Sphere(glm::vec3(0, 0, 0), 1.0, colors[random]));
	break; 
	}
	case OF_KEY_BACKSPACE:
		for (int i = 0; i < scene.size(); i++) {
			if (objSelected() && scene[i] == selected[0]) {
				scene.erase(scene.begin() + i);
			}
		}
		break;
		
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {


	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);
		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
		}
		else {
			selected[0]->position += (point - lastPoint);
		}
		lastPoint = point;
	}
}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}

	}

	for (int i = 0; i < lights.size(); i++) {
		glm::vec3 point, norm;

		//If we hit a light
	 if (lights[i]->isSelectable && lights[i]->intersect(Ray(p, dn), point, norm)) {
		hits.push_back(lights[i]);
	}
	}


	// if we selected more than one, pick nearest
	//
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bDrag = false;
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

