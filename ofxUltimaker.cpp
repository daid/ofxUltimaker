#include "ofxUltimaker.h"

ofxUltimaker::ofxUltimaker() {
    isBusy = false;
    isPrinting = false;
    temperature = 0;
}

bool ofxUltimaker::autoConnect() {
    //without params autodetect ultimaker
    listDevices();
    buildDeviceList();
    for (int k=0; k < (int)devices.size(); k++){
        if (devices[k].getDeviceName().find("usbmodem")!=string::npos) { //osx
            return connect(k);
        }
    }
}

bool ofxUltimaker::connect(int portnumber, int speed) {
    return connect(devices[portnumber].getDeviceName(), speed);
}

bool ofxUltimaker::connect(string portname, int speed) {
    //listDevices();
    
    if ((isConnected = setup(portname,speed))) {
        #if OF_VERSION_MINOR==0
            ofAddListener(ofEvents.update, this, &ofxUltimaker::update);
        #else
            ofAddListener(ofEvents().update, this, &ofxUltimaker::update);
        #endif
    }
    return isConnected;
}

void ofxUltimaker::readTemperature() {
    send("M105");
}
//    string result = request("M105");
//
//    if (isOk(result)) {
//        return ofToFloat(ofSplitString(ofSplitString(result,",")[0],":")[1]);
//    } else {
//        ofLogError() << "Problem reading Ultimaker temperature" << endl;
//        return 0;
//    }
//}
////
void ofxUltimaker::setTemperature(float temperature) { //, bool waitUtilReached) {
//    if (waitUtilReached) {
//        isBusy =
//    }
    request("M104 S"+ofToString(temperature));
    //string str = request( (waitUtilReached ? "M104" : "M109") + " S" + ofToString(temperature));
    //send("M104 S"+ofToString(temperature));
    //isBu
}
//
//void ofxUltimaker::setTemperature(float temperature) {
//
//}

void ofxUltimaker::load(string filename) {
    gcode.load(filename);
}

void ofxUltimaker::startPrint() {
    isPrinting = true;
    currentLine = 0;
    cout << "start" << endl;
    if (gcode.lines.size()==0) {
        cout << "no gcode loaded" << endl;
    } else {
        send(gcode.lines.at(0)); //send first line
    }
}

void ofxUltimaker::update(ofEventArgs &e) {
    for (int i=0; i<100; i++) {
        string str = ofxGetSerialString(*this,'\n');
        if (str!="") {

            cout << "> " << str << endl;
            
            int temperaturePos = str.find("T:");
            if (temperaturePos > -1)
            {
                temperature = ofToFloat(str.substr(temperaturePos+2));
            }

            if (isOk(str)) {
                isBusy = false;
                if (gcode.lines.size()>0 && currentLine<gcode.lines.size()) {
                    send(gcode.lines.at(currentLine));
                    currentLine++;
                }
            }
        }
    }
}

//MAX SCALE is dat boundingbox per laag meer dan bijv 0.5 mm mag toenemen.

//(boundingBox.width/(boundingBox.width-0.4));

void ofxUltimaker::stopPrint() {
    cout << "stop" << endl;
    isPrinting = false;
    isBusy = false;
    gcode.lines.clear();
}

void ofxUltimaker::setAbsolute() {
    request("G90 (absolute)");
}

void ofxUltimaker::setRelative() {
    request("G91 (relative)");
}

void ofxUltimaker::extrude(float amount, float speed) {
    send("G1 F"+ofToString(speed)+" E"+ofToString(amount));
}

void ofxUltimaker::physicalHomeXYZ() {
    send("G28 X0 Y0 Z0 (physical home)");
}

bool ofxUltimaker::isOk(string str) {
    return (str.length()>=2 && str[0]=='o' && str[1]=='k');
}

void ofxUltimaker::moveTo(float x, float y) {
    //send(ofVAArgsToString("G1 X%03f Y%03f",x,y));
}

string ofxUltimaker::request(string cmd) {
    if (!isConnected) return "";
    
    //dit moet worden vervangen door async event. Of kan het naast elkaar leven?
    send(cmd);
    string str;
    int ttl=100000;
    while (str=="" && ttl-->0) str=ofxGetSerialString(*this,'\n');
    if (str!="") {
        cout << str << endl;
        isBusy=false;
    }
    //if (isOk(str))
    return str;
}

void ofxUltimaker::send(string cmd) {
    if (!isConnected) return;
    if (isBusy) {
        cout << "Ultimaker is busy processing '"<<prevCmd<<"'. Please resend '"<<cmd<<"' again later" << endl;
    } else {
        cout << "Sending: " << cmd << endl;
        isBusy = true;
        ofxSerialWriteLine(*this,cmd);
        prevCmd = cmd;
    }
}
