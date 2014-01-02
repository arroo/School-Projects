default: clean here

run: clean all
	java Sketch
all:
	javac ./model/*.java ./view/*.java *.java
model:
	javac ./model/*.java
view:
	javac ./view/*.java
here:
	javac ./view/*.java *.java
clean:
	rm -f *.class
	rm -f ./model/*.class
	rm -f ./view/*.class
