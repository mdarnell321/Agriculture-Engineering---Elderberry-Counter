# Agriculture-Engineering---Elderberry-Counter

Note: this program is for Linux but can be ported over to windows easily

Setup:

sudo apt-get install codelite<br>
sudo apt-get install codelite-plugins<br>
sudo apt-get install -y libopencv-dev<br>
sudo apt-get install libboost-all-dev<br>
sudo apt install ffmpeg<br>


Appended linker options

-lopencv_core; -lopencv_imgcodecs;-lopencv_highgui;-lopencv_imgproc;-lopencv_photo;-lopencv_video;-lopencv_videoio;

Appended compiler options

-I/usr/include/opencv4 `pkg-config --libs --cflags opencv4`

How to use:

**1.) Export all the frames as images**

ffmpeg -i "GX010244.MP4" "out-%01d.jpg"

ffmpeg -i GX010244.MP4 \
    -vf "drawtext=fontfile=Arial.ttf: text=%{n}: x=(w-tw)/2: y=h-(2*lh): fontcolor=white: fontsize=30: box=1: boxcolor=0x00000099" \
    output.mp4

2.)Open the workspace in Codelite IDE and select agr_actual; build and run the project. On opening enter an output jpg that you would like to calibrate with.<br>
	A.)Left click to start to draw ignore bounds and then click again to finalize 	the rectangular bounds. There should visually be a box.<br>
		-Left click to delete most recently drawn ignore bound<br>
	B.)Adjust the slider values accordingly that closely matches ground truth of 	that frame and visually looks the most accurate. All frame processing will 	be based on these parameters, so they have to be close to perfect<br>


	Note: All parameters will be saved to params.txt file in local directory

3.)In the console enter the frame you would like to start at and then press enter. All windows will close and processing of all frames will begin. Data will be stored in ./count.txt.

**2.)Data post-processing<br>**
1.)Build and run agr_cleaner<br>
	A.)Copy and paste the file path of count.txt into the terminal and press 	enter.<br>
	B.)Enter the amount of frames you would like to combine through and 	average (Each set of X will be averaged, and each set will now act as a new 	‘frame’).<br>
	C.)Output will be to file count_averaged(X).txt<br>

2.)Build & run agr_shifter<br>
	A.)Specify if zeroing or appending<br>
	B.)Enter the frame to start at if appending<br>
	C.)Copy and paste the file path of averaged output into the terminal and 	press enter.<br>

	Note: This program adds/subtracts X from the frame specified and all 	frames following. Zeroing starts at first frame, and appending starts at user 	specified frame. Appending will generally be used to keep the plot curve as 	close to ground truth curve as possible as time progresses.

**3.)Build & run agr_parser<br>**
	A.)Copy and paste the finalized post processed data into the terminal and 	press enter.<br>
	B.)frame, frame_count, and time text files will be in local excel directory 	that was just created. From these files, you may copy and paste data 	directly into an excel column.<br>
