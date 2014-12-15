#include <stdlib.h>
#include <stdio.h>
#include "sgct.h"
#include <sstream>

sgct::Engine * gEngine;

//-----------------------
// function declarations
//-----------------------
void myInitOGLFun();
void myPreSyncFun();
void myDrawFun();

//for syncing variables across a cluster
void myEncodeFun();
void myDecodeFun();

void drawAxes(float size);
void drawWireCube(float size);


//-----------------------
// variable declarations
//-----------------------
sgct_utils::SGCTSphere * sphereSun = NULL;
sgct_utils::SGCTSphere * sphereEarth = NULL;
sgct_utils::SGCTSphere * sphereMoon = NULL;

//store each device's transform 4x4 matrix in a shared vector
sgct::SharedVector<glm::mat4> sharedTransforms;
sgct::SharedString sharedText;
sgct::SharedObject<size_t> sharedHeadSensorIndex(0);

//pointer to a device
sgct::SGCTTrackingDevice * devicePtr = NULL;
//pointer to a tracker
sgct::SGCTTracker * trackerPtr = NULL;

//variables to share across cluster
sgct::SharedDouble curr_time(0.0);

size_t myTextureHandle;

int main( int argc, char* argv[] )
{
	// Allocate
	gEngine = new sgct::Engine( argc, argv );

	// Bind your functions
	gEngine->setInitOGLFunction( myInitOGLFun );
	gEngine->setPreSyncFunction( myPreSyncFun );
	gEngine->setDrawFunction( myDrawFun );

	// Init the engine
	if( !gEngine->init() )
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	sgct::SharedData::instance()->setEncodeFunction( myEncodeFun );
	sgct::SharedData::instance()->setDecodeFunction( myDecodeFun );

	// Main loop
	gEngine->render();

	// Clean up
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
}

void myInitOGLFun()
{
  
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_LIGHTING );
	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	
	sgct::TextureManager::instance()->setAnisotropicFilterSize(8.0f);
	sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);
	sgct::TextureManager::instance()->loadTexure(myTextureHandle, "textureSun", "mars.png", true);
	sgct::TextureManager::instance()->loadTexure(myTextureHandle, "textureEarth", "earth.jpeg", true);
	sgct::TextureManager::instance()->loadTexure(myTextureHandle, "textureMoon", "moon.jpeg", true);

	sphereSun = new sgct_utils::SGCTSphere(0.5f, 30); 		//Solen
	sphereEarth = new sgct_utils::SGCTSphere(0.2f, 30); 		//Jorden
	sphereMoon = new sgct_utils::SGCTSphere(0.05f, 30); 		//Månen
	
	//only store the tracking data on the master node
	if( gEngine->isMaster() )
	{
		size_t index = 0;

		//allocate shared data
		for(size_t i = 0; i < sgct::Engine::getTrackingManager()->getNumberOfTrackers(); i++)
		{
			trackerPtr = sgct::Engine::getTrackingManager()->getTrackerPtr(i);

			//init the shared vector with identity matrixes
			for(size_t j=0; j<trackerPtr->getNumberOfDevices(); j++)
			{
				devicePtr = trackerPtr->getDevicePtr(j);

				if( devicePtr->hasSensor() )
				{
					sharedTransforms.addVal( glm::mat4(1.0f) );

					//find the head sensor
					if( sgct::Engine::getTrackingManager()->getHeadDevicePtr() == devicePtr )
						sharedHeadSensorIndex.setVal(index);

					index++;
				}
			}
		}
	}
}

/*
	This callback is called once per render loop iteration.
*/
void myPreSyncFun()
{
	/*
	Store all transforms in the array by looping through all trackers and all devices.

	Storing values from the tracker in the pre-sync callback will guarantee
	that the values are equal for all draw calls within the same frame.
	This prevents the application from getting different tracked data for
	left and right draws using a stereoscopic display. Remember to get
	all sensor, button and analog data that will affect the rendering in this stage.
	*/

	//only store the tracking data on the master node
	if( gEngine->isMaster() )
	{
		size_t index = 0;
		std::stringstream ss;
		curr_time.setVal( sgct::Engine::getTime() );
		/*
			Loop trough all trackers (like intersense IS-900, Microsoft Kinect, PhaseSpace etc.)
		*/
		for(size_t i = 0; i < sgct::Engine::getTrackingManager()->getNumberOfTrackers(); i++)
		{
			trackerPtr = sgct::Engine::getTrackingManager()->getTrackerPtr(i);


			/*
				Loop trough all tracking devices (like headtracker, wand, stylus etc.)
			*/
			for(size_t j = 0; j < trackerPtr->getNumberOfDevices(); j++)
			{
				devicePtr = trackerPtr->getDevicePtr(j);

				ss << "Device " << i <<  "-" << j << ": " << devicePtr->getName() << "\n";

				if( devicePtr->hasSensor() )
				{
					sharedTransforms.setValAt( index, devicePtr->getWorldTransform() );
					index++;

					double trackerTime = devicePtr->getTrackerDeltaTime();
					ss << "     Sensor id: " << devicePtr->getSensorId()
						<< " freq: " << (trackerTime <= 0.0 ? 0.0 : 1.0/trackerTime) << " Hz\n";

					ss << "\n     Pos\n"
						<< "          x=" << devicePtr->getPosition().x << "\n"
						<< "          y=" << devicePtr->getPosition().y << "\n"
						<< "          z=" << devicePtr->getPosition().z << "\n";

					ss << "\n     Rot\n"
						<< "          rx=" << devicePtr->getEulerAngles().x << "\n"
						<< "          ry=" << devicePtr->getEulerAngles().y << "\n"
						<< "          rz=" << devicePtr->getEulerAngles().z << "\n";

				}

				if( devicePtr->hasButtons() )
				{
					ss << "\n     Buttons\n";

					for(size_t k=0; k < devicePtr->getNumberOfButtons(); k++)
					{
						ss << "          Button " << k << ": " << (devicePtr->getButton(k) ? "pressed" : "released") << "\n";
					}
				}

				if( devicePtr->hasAnalogs() )
				{
					ss << "\n     Analog axes\n";

					for(size_t k=0; k < devicePtr->getNumberOfAxes(); k++)
					{
						ss << "          Axis " << k << ": " << devicePtr->getAnalog(k) << "\n";
					}
				}

				ss << "\n";
			}
		}

		//store the string stream into the shared string
		sharedText.setVal( ss.str() );
	}
}

/*
	This callback can be called several times per render loop iteration.
	Using a single viewport in stereo (3D) usually results in refresh rate of 120 Hz.
*/
void myDrawFun()
{
  
	//constants
	double speed = 25.0;

	//devicePtr = trackerPtr->getDevicePtr(size_t(0));
	//getNumberOfDevices();

		/*
	if (devicePtr->getPosition().x <= 0.5 && devicePtr->getPosition().y <= 0.5 && devicePtr-> getPosition().z <= 0.5 )
	{
        glColor3f(1.0f,0.0f,0.0f);
	}
	*/
	
	// loop trough all the devices to find the wand
	for(size_t j = 0; j < trackerPtr->getNumberOfDevices(); j++) {
	    
	    // we have found the wand if this is true
	    if( j != sharedHeadSensorIndex.getVal() ) {
	      
	      //store the wand in the wandPtr
	      wandPtr = trackerPtr->getDevicePtr(j);
	      
	      // get the coordinates (x,y,z) of the wand
	      glm::vec3 wandPosition = wandPtr->getPosition();
	      
	      // get the Euler angeles around the x,y,z axis.
	      glm::vec3 wandRotationEuler = wandPtr->getEulerAngles();
	      
	      
	      // ---------------------------//
	      //Calculate the distance between the Wand and the Object (Ths SUN in this case)
	      
	      // the origin of the sun --> constant
	      glm:vec3 sunPosition = glm::vec3(0.f,0.f,0.f);
	      
	      // Already have the wandPosition
	      
	      // OUR b vector --> Distance from the wand to the sun. --> want the vector to point to the sun --> (0,0,0) - (a,b,c) = negative
	      glm:vec3 b = sunPosition - wandPosition;
	      
	      
	      // ---------------------------//
	      //Calculate the where to wand is pointing
	      
	      //4x4 homogeneous rotation matrix from the euler angles
	      glm::mat4 rotMat = glm::yawPitchRoll(wandRotationEuler);
	      
	      glm::vec4 negativeZ = glm::vec4(0.f,0.f,-1.f,0.f);
	      
	      glm::vec4 a = rotMat*negativeZ;
	      
	      // ---------------------------//
	      //Calculate the vector d, which is the distance between origin of the SUN and the closest point from the line.
	      
	      //cast to vec3
	      glm::vec3 a2 = glm::vec3(a);
	      
	      //calulate the new vector
	      glm::vec3 ny = glm::dot(a2, b);
	      
	      glm::vec3 d = b-ny;
	      
	      
	      
	      if ( d < SUN_RADIUS ) {
		
		//change some color?? or set some boolean to change the color.
		
	      }
	      
	      
	      
	      
	    }
	  
	  
	  
	  
	}
	
	
	//bind Textures 
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("textureSun") );
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("textureEarth") );
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("textureMoon") );

	// SUN
	glPushMatrix(); //set where to start the current object transformations
	glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	sphereSun->draw();
	glPopMatrix();
	
	// EARTH
	glPushMatrix();
	glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	glTranslatef(1.0, 0.0f,0.0f);
	glRotated(curr_time.getVal() * speed*2, 0.0, -1.0, 0.0);
	sphereEarth->draw();
	glPopMatrix();
	
	// MOON
	glPushMatrix();	
	glRotated(curr_time.getVal() * speed*4, 0.0, -1.0, 0.0);
	glTranslatef(0.3, 0.1f, 0.0f);
	sphereMoon->draw();
	glPopMatrix();

	//glColor3f(1.0f,0.0f,0.0f);



	
// 	//draw some yellow cubes in space
// 	for( float i=-0.5f; i<=0.5f; i+=0.2f)
// 		for(float j=-0.5f; j<=0.5f; j+=0.2f)
// 		{
// 			glPushMatrix();
// 			glTranslatef(i, j, 0.0f);
// 			glColor3f(1.0f,1.0f,0.0f);
// 			drawWireCube(0.04f);
// 			glPopMatrix();
// 		}


	//draw a cube and axes around each wand
	for(size_t i = 0; i < sharedTransforms.getSize(); i++)
	{
		if(i != sharedHeadSensorIndex.getVal())
		{
			glLineWidth(2.0);

			glPushMatrix();

			glMultMatrixf( glm::value_ptr( sharedTransforms.getValAt( i ) ) );

			glColor3f(0.5f,0.5f,0.5f);
			drawWireCube(0.1f);

			drawAxes(0.1f);

			//draw pointer line
			glBegin(GL_LINES);
			glColor3f(1.0f,1.0f,0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, -5.0f);
			glEnd();

			glPopMatrix();
		}
	}

	//draw text
	float textVerticalPos = static_cast<float>(gEngine->getActiveWindowPtr()->getYResolution()) - 100.0f;
	int fontSize = 12;

	glColor3f(1.0f, 1.0f, 1.0f);
	sgct_text::print(sgct_text::FontManager::instance()->getFont( "SGCTFont", fontSize ),
		120.0f, textVerticalPos,
		sharedText.getVal().c_str() );
}

void myEncodeFun()
{
	sgct::SharedData::instance()->writeVector( &sharedTransforms );
	sgct::SharedData::instance()->writeString( &sharedText );
	sgct::SharedData::instance()->writeObj( &sharedHeadSensorIndex );
}

void myDecodeFun()
{
	sgct::SharedData::instance()->readVector( &sharedTransforms );
	sgct::SharedData::instance()->readString( &sharedText );
	sgct::SharedData::instance()->readObj( &sharedHeadSensorIndex );
}

void drawAxes(float size)
{
	glLineWidth(2.0);
	glBegin(GL_LINES);

	//x-axis
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(size, 0.0f, 0.0f);

	//y-axis
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, size, 0.0f);

	//z-axis
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, size);

	glEnd();
}

void drawWireCube(float size)
{
	//bottom
	glBegin(GL_LINE_STRIP);
	glVertex3f( -size, -size, -size);
	glVertex3f( size, -size, -size);
	glVertex3f( size, -size, size);
	glVertex3f( -size, -size, size);
	glVertex3f( -size, -size, -size);
	glEnd();

	//top
	glBegin(GL_LINE_STRIP);
	glVertex3f( -size, size, -size);
	glVertex3f( size, size, -size);
	glVertex3f( size, size, size);
	glVertex3f( -size, size, size);
	glVertex3f( -size, size, -size);
	glEnd();

	//sides
	glBegin(GL_LINES);
	glVertex3f( -size, -size, -size);
	glVertex3f( -size, size, -size);

	glVertex3f( size, -size, -size);
	glVertex3f( size, size, -size);

	glVertex3f( size, -size, size);
	glVertex3f( size, size, size);

	glVertex3f( -size, -size, size);
	glVertex3f( -size, size, size);
	glEnd();
}
