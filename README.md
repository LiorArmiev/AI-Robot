# AI-Robot

This small project was created for AI confarance on 7/8/2019.
Otto is a 3D printed robot that everyone can make (even without 3d printer) 
The perpose of this demo was to show how to combine a toy robot with Azure cognitive services.

If you want to create your own robot go to https://www.ottodiy.com/
I created a c++ program for the robot that you can use with your Arduino and upload it as is.
its importent to note that i used in my project an mp3 (mp3-tf-16p) insted of the buzzer that is in the original project.

In my Git you will find the Robot project file and the Python program that is using face recognition and detection.

For the Python project you will need to have CV2 installed

## Prerequsetis
1. Install Pyhon and VS Code
2. Install OpenCV for Python
3. Train Face recognition model
    this is simple and done by using the Train-Face-Model.py file under Python folder.
    all you need to do is to create a folder with the name Images
    under that folder create a Named folder with the names of the people you want to recognize and put
    face images in the named folder of the person you want. the code is using Jpg umages but you can cahnge it in the 
    code if you want.
    Run the Train-Face-Model.py script and it will create trainner.yml file and a labels.pickle file
    you need the two files for face recognition in the main script.
    Note: If you change folders later take the pickle and yml files with you.

4. Create Azure cognitive Services under you Azure portal.
   https://docs.microsoft.com/en-us/azure/cognitive-services/cognitive-services-apis-create-account
   
5. Make changes to the main program with the new Cognitive Services Key and Region.
6. connect your Otto robot to the Bluetooth of your computer
7. Run the program

    
