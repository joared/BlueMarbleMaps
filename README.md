# BlueMarble Map

<img src="readme/CompleteGeodata.png" title="Entire eart from Goggle Sattelit pikecture" alt="faqqa you" width="500"/>

Emscripten:
cd external/emsdk
./emsdk install 3.1.45
./emsdk activate 3.1.45
source ./emsdk_env.sh
export EM_NODE_JS=$(which node) 

( forgot when to use if needed : export EMSDK_NODE=/usr/bin/node (fallback))

cd ../../
mkdir build_web
cd build_web
emcmake cmake ..
cmake --build .

copy ExampleImgui.html to bin folder (cant compile it, old version) 

cmd in bin folder: python3 -m http.server 8080 --bind 0.0.0.0 
