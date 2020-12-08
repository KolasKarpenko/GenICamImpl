#include <iostream>

#include <GL/glut.h>

#include <CDevice.h>
#include <ImageData.h>

static std::mutex imageDataMutex;
static gevdevice::ImageData imageData;

#define TIMERMSECS 30

void animate(int value) {
	glutTimerFunc(TIMERMSECS, animate, 1);
	glutPostRedisplay();
}

void display() {
	std::lock_guard<std::mutex> lock(imageDataMutex);

	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

	switch (imageData.PixelType)
	{
		case gevdevice::GVSP_PIXEL_TYPES::GVSP_PIX_MONO8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, imageData.SizeX, imageData.SizeY, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, imageData.Bitmap.data());
			break;
		case gevdevice::GVSP_PIXEL_TYPES::GVSP_PIX_RGB8_PACKED:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageData.SizeX, imageData.SizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData.Bitmap.data());
			break;
		default:
			break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);                                  // Draw A Quad
        glVertex3f(-1.0f, 1.0f, 0.0f);                  // Top Left
        glTexCoord2i( 1, 0 );
        glVertex3f( 1.0f, 1.0f, 0.0f);                  // Top Right
        glTexCoord2i( 1, 1 );
        glVertex3f( 1.0f,-1.0f, 0.0f);                  // Bottom Right
        glTexCoord2i( 0, 1 );
        glVertex3f(-1.0f,-1.0f, 0.0f);                  // Bottom Left
        glTexCoord2i( 0, 0 );
     glEnd();

	glutSwapBuffers(); // Render now
}

int main(int argc, char* argv[])
{
	gevdevice::CDevice::InitSystem();

	auto connectionsPromise = gevdevice::CDevice::FindAll();
	std::vector<gevdevice::UdpPort::Connection> connections;
	connectionsPromise->Result(connections);

	std::cout << "Devices in network: " << connections.size() << std::endl;

	if (connections.empty()) {
		return 0;
	}

	int imgCount = 0;
	auto start = std::chrono::high_resolution_clock::now();

	gevdevice::CDevice::TFrameCallBack onFrame = [&imgCount, &start](const gevdevice::FrameData& frame) {
		std::lock_guard<std::mutex> lock(imageDataMutex);
		imageData = frame;

		if (imageData.PixelType & GVSP_PIX_OCCUPY8BIT) {
			imageData = gevdevice::ImageData::Demosaic(imageData);
		}

		imgCount++;

		auto curr = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(curr - start).count();

		if (duration > 1000) {
			start = curr;

			std::cout << "FPS " << imgCount << std::endl;
			imgCount = 0;
		}
	};

	gevdevice::CDevice::TErrorCallBack onError = [](gevdevice::CDevice::Error errType, const std::string& err) {
		std::cout << gevdevice::CDevice::ErrorTypeToString(errType) << " " << err << std::endl;
	};

	std::cout << "connecting to device: " << gevdevice::UdpPort::IpAddressToString(connections[0].cameraAddr) << std::endl;

	auto camera = gevdevice::CDevice::Create(connections[0], onFrame, onError);
	std::cout << "device mac address: " << gevdevice::UdpPort::MacAddressToString(camera->GetMacAddress()) << std::endl;

	camera->Connect();
	if (!camera->IsConnected()) {
		return 0;
	}

	GenApi::CCommandPtr acqStart = camera->GetGenApi()._GetNode("AcquisitionStart");

	GenApi_3_2::CNodeMapRef api = camera->GetGenApi();
	GenApi_3_2::NodeList_t nodes;
	api._GetNodes(nodes);

	for (GenApi_3_2::INode* node : nodes) {
		if (node->GetAccessMode() == GenApi_3_2::EAccessMode::NA ||
			node->GetAccessMode() == GenApi_3_2::EAccessMode::NI ||
			node->GetAccessMode() == GenApi_3_2::EAccessMode::WO
			) {
			continue;
		}

		std::cout << node->GetName() << " : ";

		GenApi::CCommandPtr comandPtr = node;
		if (comandPtr.IsValid()) {
			std::cout << "__comand__" << std::endl;
			continue;
		}

		GenApi::CStringPtr strPtr = node;
		if (strPtr.IsValid()) {
			std::cout << strPtr->GetValue() << std::endl;
			continue;
		}

		GenApi::CBooleanPtr boolPtr = node;
		if (boolPtr.IsValid()) {
			std::cout << boolPtr->GetValue() << std::endl;
			continue;
		}

		GenApi::CIntegerPtr intPtr = node;
		if (intPtr.IsValid()) {
			std::cout << intPtr->GetValue() << std::endl;
			continue;
		}

		GenApi::CFloatPtr floatPtr = node;
		if (floatPtr.IsValid()) {
			std::cout << floatPtr->GetValue() << std::endl;
			continue;
		}
	}

	GenApi::CIntegerPtr width = camera->GetGenApi()._GetNode("Width");
	GenApi::CIntegerPtr height = camera->GetGenApi()._GetNode("Height");

	int64_t winWidth = 1000;
	int64_t winHeight = 1000;

	if (width.IsValid()) {
		winWidth = width->GetValue();
		std::cout << "Width " << winHeight << std::endl;
	}

	if (height.IsValid()) {
		winHeight = height->GetValue();
		std::cout << "Height " << winHeight << std::endl;
	}

	GenApi::CEnumerationPtr triggerSelector = api._GetNode("TriggerSelector");
	if (triggerSelector.IsValid()){
		triggerSelector->FromString("FrameStart");
	}

	GenApi::CEnumerationPtr triggerMode = api._GetNode("TriggerMode");
	if (triggerMode.IsValid()){
		triggerMode->FromString("Off");
	}

	if (acqStart.IsValid()) {
		acqStart->Execute();
		std::cout << "AcquisitionStart" << std::endl;

		glutInit(&argc, argv);                 // Initialize GLUT
		glutInitWindowSize(winWidth / 2, winHeight / 2);   // Set the window's initial width & height
		glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
		glutCreateWindow("Test"); // Create a window with the given title

		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
		glEnable(GL_TEXTURE_2D);

		glutDisplayFunc(display); // Register display callback handler for window re-paint
		glutTimerFunc(TIMERMSECS, animate, 0);

		glutMainLoop();
	}
	else {
		std::cout << "AcquisitionStart command is not exists" << std::endl;
	}


	GenApi::CCommandPtr acqStop = camera->GetGenApi()._GetNode("AcquisitionStop");

	if (acqStop.IsValid()) {
		acqStop->Execute();
		std::cout << "AcquisitionStop" << std::endl;
	}


	gevdevice::CDevice::FinishSystem();

	return 0;
}