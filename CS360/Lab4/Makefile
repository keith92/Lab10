webtest: webtest.cpp
	g++ -o webtest webtest.cpp -std=c++11

run: webtest.cpp
	./webtest http://www.mclement.us:80/test.html 15

rund: webtest.cpp
	./webtest http://www.mclement.us:80/test.html -d 15

runmy: webtest.cpp
	./webtest http://ec2-54-218-67-46.us-west-2.compute.amazonaws.com:3008/test1.html 100

runmyd: webtest.cpp
	./webtest http://ec2-54-218-67-46.us-west-2.compute.amazonaws.com:3008/test1.html -d 100
