#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

/**
   Detects faces, eyes and smiles, outlines them, blurs background and sharpens each face

   @param image - Mat object that holds a frame from the web-cam
   @param gray_scale_image Mat object that holds the grey scale converted frame from web-cam
   @param face_cascade Haar Cascade face classifier
   @param eye_cascade Haar Cascade eye classifier
   @param smile_cascade Haar Cascade smile classifier
   @param scaling_factor Scaling factor used to reduce computational load
   @param color_array Array of various colors
   @return void
*/
void detect_and_process(Mat& image, const Mat& gray_scale_image,
                        CascadeClassifier& face_cascade,
                        CascadeClassifier& eye_cascade,
                        CascadeClassifier& smile_cascade,
                        double scaling_factor, Scalar color_array[]);

int main() {
    cout << "Starting detection program" << endl;

    double scaling_factor = 2; // Used for scaling image
    CascadeClassifier face_cascade; // Face Haar Cascade
    CascadeClassifier eye_cascade; // Eye Haar Cascade
    CascadeClassifier smile_cascade; // Smile Haar Cascade

    face_cascade.load("HaarCascade_Models/haarcascade_frontalface_default.xml");
    eye_cascade.load("HaarCascade_Models/haarcascade_eye_tree_eyeglasses.xml");
    smile_cascade.load("HaarCascade_Models/haarcascade_smile.xml");

    Scalar red = Scalar(0, 0, 255);
    Scalar blue = Scalar(255, 0, 0);
    Scalar green = Scalar(0, 255, 0);
    Scalar white = Scalar(255, 255, 255);
    Scalar colors [] = {red, blue, green, white};

    // Opens '0th' web cam on the system
    VideoCapture capture(0);

    // Terminate if not opened
    if (!capture.isOpened()) {
        return -1;
    }

    while (true) {
        Mat frame;  // Holds each frame from the capture
        Mat gray_scale_frame; // Holds grey scale converted frame

        capture >> frame;
        // Convert frame to grayscale
        cvtColor(frame, gray_scale_frame, COLOR_BGR2GRAY);
        // Resize width and height to reduce computational load and decrease lag on web cam
        resize(gray_scale_frame, gray_scale_frame, Size(gray_scale_frame.size().width / scaling_factor,
                                                        gray_scale_frame.size().height / scaling_factor));
        // Detect faces, blur background and sharpen faces
        detect_and_process(frame, gray_scale_frame, face_cascade, eye_cascade,
                           smile_cascade, scaling_factor, colors);

        putText(frame,"Face Detection", Point(5, 20),FONT_HERSHEY_COMPLEX_SMALL,1.0,
                colors[1],1
        );

        // show the frame which now contains outlines faces, text and the blurred background
        imshow("Camera Frame", frame);

        // if key is pressed break the loop
        if (waitKey(10) > 0) {
            break;
        }
    }

    return 0;
}


void detect_and_process(Mat& image, const Mat& gray_scale_image,
                        CascadeClassifier& face_cascade,
                        CascadeClassifier& eye_cascade,
                        CascadeClassifier& smile_cascade,
                        double scaling_factor, Scalar color_array[]){

    vector <Rect> faces; // holds detected faces
    vector<Mat> sub_images; // holds Mat objects that contain only the face not background

    // Use the Haar Cascade classifier to find faces in the grey_scale frame
    face_cascade.detectMultiScale(gray_scale_image, faces, 1.1, 5, 0, Size(30, 30));

    for (size_t i = 0; i < faces.size(); i++) {

        Rect rect_with_face(cvRound(faces[i].x* scaling_factor),
                            cvRound(faces[i].y* scaling_factor),
                            faces[i].width* scaling_factor,
                            faces[i].height* scaling_factor);

        // Prevents crashing if the rect_with_face is outside the bounds of the image itself
        //      The statment 'bool is_rect_inside' was taken from stackoverflow answer listed below
        //      https://stackoverflow.com/questions/29120231/how-to-verify-if-rect-is-inside-cvmat-in-opencv
        bool is_rect_inside = (rect_with_face & Rect(0, 0, image.cols, image.rows)) == rect_with_face;
        if (!is_rect_inside) {
            cout << "Outside Range\n";
            continue;
        }

        Mat face_sub_img; // holds ROI of image that contains the face
        Mat sharpening_kernel; // holds kernel used to sharpen image

        // Isolate the area with the face and sharpen it using a common sharpening kernel
        face_sub_img = image(rect_with_face);
        sharpening_kernel = (Mat_<double>(3, 3) << -1, -1, -1, -1, 9, -1, -1, -1, -1);
        filter2D(face_sub_img, face_sub_img, -1, sharpening_kernel);
        sub_images.push_back(face_sub_img);

        // Draw rectangle for the face
        rectangle(image, rect_with_face, color_array[0], 3, 8, 0);

        // Detect eyes
        vector<Rect> eyes;
        eye_cascade.detectMultiScale(gray_scale_image, eyes, 1.1, 5, 0, Size(30, 30));

        for (size_t j = 0; j < eyes.size(); j++) {
            // Draw a rect around each eye in blue
            rectangle(image, Point(cvRound(eyes[j].x * scaling_factor),
                                   cvRound(eyes[j].y * scaling_factor)),
                      Point(cvRound((eyes[j].x + eyes[j].width) * scaling_factor),
                            cvRound((eyes[j].y + eyes[j].height) * scaling_factor)),
                      color_array[1], 3, 8, 0);
        }

        // Detect smiles only in the area that the face was detected
        Mat smile_frame;
        Mat gray_scale_smile_frame;
        smile_frame = image(rect_with_face);
        // Convert frame to grayscale
        cvtColor(smile_frame, gray_scale_smile_frame, COLOR_BGR2GRAY);
        vector <Rect> smiles;
        // Increase the minNeighbers for less false positives
        smile_cascade.detectMultiScale(gray_scale_smile_frame, smiles, 1.8, 20, 0,
                                       Size(30, 30));

        Rect rect_for_label(cvRound(faces[i].x * scaling_factor),cvRound(faces[i].y * scaling_factor),
                            120 * scaling_factor, 15 * scaling_factor);
        rectangle(image,rect_for_label,color_array[0],-1,8,0);
        // Flag whether or not a smile was found
        if (!smiles.empty()) {
            putText(image, "Smile Detected", Point(faces[i].x * scaling_factor + 3, faces[i].y * scaling_factor + 20),
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0,
                    color_array[2], 1
            );
        }
        else {
            putText(image, "No Smile Detected", Point(faces[i].x * scaling_factor + 3, faces[i].y * scaling_factor + 20),
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0,
                    color_array[3], 1);
        }
    }

    // Blur the background but keep each face there
    Mat blurred_background; // holds background with Gaussian Blur applied
    blurred_background = image.clone();
    GaussianBlur(blurred_background, blurred_background, Size(0, 0), 10, 10);
    for (size_t i = 0; i < faces.size(); i++) {
        Rect rect_with_face(cvRound(faces[i].x* scaling_factor),
                            cvRound(faces[i].y* scaling_factor),
                            faces[i].width* scaling_factor,
                            faces[i].height* scaling_factor);
        // Copies each sharpened face onto the blurred background
        sub_images[i].copyTo(blurred_background(rect_with_face));
    }

    blurred_background.copyTo(image);

    string face_count_display = "Number of faces found: " + to_string(faces.size());
    putText(image, face_count_display, Point(5, 40), FONT_HERSHEY_COMPLEX_SMALL,1.0,
            color_array[1],1
    );
}
