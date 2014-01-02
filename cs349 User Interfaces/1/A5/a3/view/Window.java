package view;

import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.io.*;
import java.lang.Math;
import java.util.*;

import javax.swing.*;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import model.*;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/*

    this is a canvas to draw and animate Things on

*/


public class Window extends JComponent {

    public static final int DRAW   = 1; // some constants to keep state
    public static final int ERASE  = 2;
    public static final int SELECT = 3;
    public static final int MOVE   = 4;
    
    int                mode,               // state of what's happening
                       currentX, currentY, // coords of mouse
                       oldX, oldY,         // coords of mouse for previous frame
                       currentFrame,       // current frame that is rendered
                       numFrames,          // maximum number of frame
                       fps,                // fps of the animation
                       wX, wY;             // size of the window
    
	Image              image;              // image to draw on
	
	boolean            inside,             // is the cursor on the image?
	                   moving;             // is the mouse currently dragging?
    
	Rectangle2D        selectBox;          // used for selecting Things to animate
	Thing              selectThing;
	Point2D            selectPoint;
	ArrayList<Integer> selectedThings;
	
	ArrayList<Thing>   things;             // all of the objects to animate
	ArrayList<Color>   colours;            // the colours of the objects
	Color              drawColour;         // current colour to draw new objects as
	
	Graphics2D         graphics2D;
	Thing              newthing;           // the newest object created
    javax.swing.Timer  t;                  // to interact with the timer
    JSlider            slide;              // to interact with the slider
    JFileChooser       choose;             // to save files
    
    
    // Window constructor takes in fps, seconds of animation, window size, a timer and a slider
	public Window(int f, int sec, final int windowX, final int windowY, final javax.swing.Timer timer, final JSlider sl) {
	    
        // set up variables based on passed in variables
        slide = sl;
        wX = windowX;
        wY = windowY;
        t = timer;
        inside = false;
        moving = false;
        numFrames = f * sec;
        fps = f;
        currentFrame = 0;
        choose = new JFileChooser();
        things = new ArrayList<Thing>();
        colours = new ArrayList<Color>();
        drawColour = new Color(0);

        nowDrawing(); // drawing is the default mode
        
        setDoubleBuffered(true); // for faster repainting
        
		// set up all of the different actions for animating
		addMouseListener(new MouseAdapter() {
		
			public void mousePressed(MouseEvent e) {
			
			    t.stop(); // stop animating
			    
				oldX = e.getX(); // get coords for changing mouse positions
				oldY = e.getY();
				
				switch (mode) {
				  case DRAW: // get a new Thing ready to be drawn
				    newthing = new Thing(oldX,oldY,currentFrame,numFrames);
				    colours.add(new Color(
				        drawColour.getRed(),
				        drawColour.getGreen(),
				        drawColour.getBlue()
				    ));
				    things.add(newthing);
				    newthing.add(oldX,oldY);
				    break;
				  case ERASE: // "erase" everything within an 8x8 box
				    for(int i=0;i<things.size();i++){
				        if(things.get(i).touches(oldX,oldY,currentFrame))
				            things.get(i).setVisibility(currentFrame, false);
				    }
				    break;
				  case SELECT: // create a Thing to select with
				    selectThing = new Thing(oldX,oldY,currentFrame,numFrames);
				    selectBox = null;
				    break;
				  case MOVE: // move a selection of Things to create animation
				    assert(selectBox != null); // should not get here without
				                               // properly selecting anything
				    if (selectBox.contains(oldX,oldY) == false) { // you missed the box,
				        mode = SELECT;                            // select new Things
				        mousePressed(e);
				    } else { // start animating!
				        selectPoint = new Point2D.Double(oldX,oldY); // the selection anchor
				        moving = true;
				        t.start();
				    }
				    break;
				}
				repaint();
			}
			
			// releasing the mouse mainly has to do with selecting and animating
			public void mouseReleased(MouseEvent e) {
			    switch(mode) {
			      case SELECT: // create a box around all of the Things selected,
			                   // to show what is available for animating
			        selectedThings = new ArrayList<Integer>(); // the index in things of selected Things
			        Polygon p = new Polygon( // any Thing totally inside this polygon is selected
			            selectThing.getXs(),
			            selectThing.getYs(),
			            selectThing.getN()
			        );
			        double minX = windowX, // used for sizing the box around
			               minY = windowY, // every Thing that is selected
			               maxX = 0,
			               maxY = 0;
			        
			        for(int i = 0; i < things.size(); i++) {
			            double tnX = windowX,
			                   tnY = windowY,
			                   txX = 0,
			                   txY = 0;
			            if (!things.get(i).getVisibility(currentFrame)) // ignore invisible Things
			                continue;
			            boolean containsAll = true;
			            Point2D [] testPoints = things.get(i).getPoints(currentFrame);
			            for(int j = 0; j < testPoints.length; j++) {
			                tnX = Math.min(tnX,testPoints[j].getX());
			                tnY = Math.min(tnY,testPoints[j].getY());
			                txX = Math.max(txX,testPoints[j].getX());
			                txY = Math.max(txY,testPoints[j].getY());
			                if(p.contains(testPoints[j]) == false) { // any point not contained means
			                    containsAll = false;                 // the Thing is not selected
			                    break;
			                }
			            }
			            if (containsAll == true) {
			                minX=Math.min(minX,tnX);
			                minY=Math.min(minY,tnY);
			                maxX=Math.max(maxX,txX);
			                maxY=Math.max(maxY,txY);
			                selectedThings.add(i); // add the index of the Thing selected
			            }
			        }
			        
			        if (!selectedThings.isEmpty()) { // if any Thing is selected
			            selectBox = new Rectangle2D.Double( // make a box around it
			                minX,
			                minY,
			                maxX - minX,
			                maxY - minY
			            ).getBounds2D();
			            mode = MOVE; // now start animating
			        } else {
			            selectBox = null;
			        }
			        selectThing = null;
			        break;
			      case MOVE:
			        t.stop(); // stop animating once the mouse is released
			        moving = false;
			        break;
			      case DRAW:
			      case ERASE:
			        selectedThings = null;
			      default:
			        break;
			    }
			    
			    t.stop();
			    repaint();
			}
			
			// these two functions are for when in ERASE mode the erasing box
			// appears and disappears as appropriate
			public void mouseExited(MouseEvent e){
			    inside = false;
			    repaint();
			}
			public void mouseEntered(MouseEvent e){
			    inside = true;
			}
		});
		
		addMouseMotionListener(new MouseMotionAdapter() {
		    public void mouseMoved(MouseEvent e) {
		        currentX = e.getX(); // update mouse position trackers
		        currentY = e.getY();
		        if (mode==ERASE) // only need to repaint if the erasing
		            repaint();   // box needs drawing
		        oldX = currentX;
		        oldY = currentY;
		    }
		    
			public void mouseDragged(MouseEvent e) { // animating & drawing & erasing
				currentX = e.getX();
				currentY = e.getY();
				if (graphics2D != null)
				    switch(mode) {
				      case DRAW: // make the newest Thing bigger
				        newthing.add(currentX, currentY);
				        break;
				      case ERASE: // erase whatever is near enough to the cursor
				        for(int i = 0; i < things.size(); i++){
				            if(things.get(i).touches(oldX,oldY,currentFrame))
				                things.get(i).setVisibility(currentFrame, false);
				        }
				        break;
				      case SELECT: // same as drawing
				        if (selectThing==null) // should never happen but does and not enough time to find out
				                               // why since this seems to make it stable
				            selectThing = new Thing(currentX,currentY,currentFrame, currentFrame+1);
				        selectThing.add(currentX,currentY);
				        break;
				      case MOVE: // animate!
				        assert(selectBox != null); // should never get here
				                                   // without things being selected
				        selectBox.setRect( // move the selection box as well
				            selectBox.getX() - oldX + currentX,
				            selectBox.getY() - oldY + currentY,
				            selectBox.getWidth(),
				            selectBox.getHeight()
				        );
				        // call the transofrming function for each selected Thing
				        for(int i = 0;selectedThings != null && i < selectedThings.size(); i++) {
				            things.get(selectedThings.get(i)).move(
				                currentX - oldX,
				                currentY - oldY,
				                currentFrame
				            );
				        }
				        break;
			    }
			    
				repaint();
				oldX = currentX;
				oldY = currentY;
			}
		});
	}
	
	// a basic resetting function
	private void now() {
	    t.stop();
	    selectThing = null;
	    selectBox = null;
	    mode = (mode == MOVE ? SELECT : mode);
	    repaint();
	}
	
	// change mode to DRAW
    public void nowDrawing(){
        mode = DRAW;
        now();
    }
    
	// change mode to ERASE
    public void nowErasing(){
        mode = ERASE;
        now();
    }
    
	// change mode to SELECT
    public void nowSelecting(){
        mode = SELECT;
        now();
    }
    
	// play the animation
    public void nowPlaying(){
        now();
        t.start();
    }
    
    // pause the animation
    public void nowPausing(){
        now();
    }
    
    // add a single frame to the animation
    public void add() {
        slide.setMaximum(slide.getMaximum() + 1);
        for(int i = 0; i < things.size(); i++)    // give each Thing
            things.get(i).addFrame(currentFrame); // a new frame
    }
    
    // repaint the component
	public void paintComponent(Graphics g) {
		if (image == null) { // make the canvas
			image = createImage(getSize().width, getSize().height);
			graphics2D = (Graphics2D) image.getGraphics();
			graphics2D.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
					RenderingHints.VALUE_ANTIALIAS_ON);
		}
		clear(); // clear the canvas
		for (int i = 0; i < things.size(); i++) { // draw all of the Things where they
		                                          // should be for the current frame
		    if(things.get(i).getVisibility(currentFrame)) {
    		    Point2D [] drawPoints = things.get(i).getPoints(currentFrame);
    		    int [] xs = new int[drawPoints.length];
    		    int [] ys = new int[drawPoints.length];
    		    for(int j = 0; j < drawPoints.length; j++) {
    		        xs[j] = (int)Math.round(drawPoints[j].getX());
    		        ys[j] = (int)Math.round(drawPoints[j].getY());
    		    }
    		    graphics2D.setPaint(colours.get(i));
    		    graphics2D.drawPolyline(xs,ys,drawPoints.length);
    		}
		}
		
		graphics2D.setPaint(Color.black);
		if (mode == ERASE && inside) // draw the erasing square
            graphics2D.drawRect(currentX-4,currentY-4,8,8);
		if (mode == SELECT && selectThing != null) // draw the selection Thing
		    graphics2D.drawPolyline(selectThing.getXs(),selectThing.getYs(),selectThing.getN());
		if(selectBox!=null) // draw the selection box
		    graphics2D.draw(selectBox);
		
		g.drawImage(image, 0, 0, null);
	}

	public void clear() { // clear the canvas
		graphics2D.setPaint(Color.white);
		graphics2D.fillRect(0, 0, getSize().width, getSize().height);
		graphics2D.setPaint(Color.black);
	}
	
	// pick a frame to display
	public void pickFrame(int i){
	    if (moving == false && mode == MOVE) { // not moving but in MOVE mode
	        selectBox = null;
	        selectedThings = null;
	        mode = SELECT;
	    } else if (mode == MOVE && selectedThings != null && selectedThings.size() > 0) {
	        for(int j = 0; j < selectedThings.size(); j++) {
	            things.get(selectedThings.get(j)).move(0, 0, currentFrame);
	            if (things.get(selectedThings.get(j)).getVisibility(i) == false) // moving and invisible
	                things.get(selectedThings.get(j)).setVisibility(i,true);
	        }
	    }
	    if(i < numFrames) // just in case
    	    currentFrame = i;
    	repaint();
	}
	
	// save the file as an xml document
	public void goodSave(File f) {
	    try {
	        DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
	        DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
	        Document doc = docBuilder.newDocument();
	        
	        Element rootElement = doc.createElement("animation");
	        doc.appendChild(rootElement);
	        
	        Element next = doc.createElement("fps");
	        next.appendChild(doc.createTextNode(Integer.toString(fps)));
	        rootElement.appendChild(next);
	        
	        next = doc.createElement("frames");
	        next.appendChild(doc.createTextNode(Integer.toString(numFrames)));
	        rootElement.appendChild(next);
	        
	        Element size = doc.createElement("size");
	        rootElement.appendChild(size);
	        next = doc.createElement("X");
	        next.appendChild(doc.createTextNode(Integer.toString(wX)));
	        size.appendChild(next);
	        next = doc.createElement("Y");
	        next.appendChild(doc.createTextNode(Integer.toString(wY)));
	        size.appendChild(next);
	        
	        // save the Things
	        for (int i = 0; i < things.size(); i++){
	            Element thing = doc.createElement("thing");
	            rootElement.appendChild(thing);
	            
	            Element base = doc.createElement("colour");
	            thing.appendChild(base);
	            base.setAttribute("R",Integer.toString(colours.get(i).getRed()));
	            base.setAttribute("G",Integer.toString(colours.get(i).getGreen()));
	            base.setAttribute("B",Integer.toString(colours.get(i).getBlue()));
	            
	            base = doc.createElement("base");
	            thing.appendChild(base);
	            Point2D [] p = things.get(i).getPoints(0);
	            for(int j=0;j<p.length;j++){
	                next = doc.createElement("x");
	                next.setAttribute("num",Integer.toString(j));
	                next.appendChild(doc.createTextNode( Integer.toString((int)p[j].getX()) ));
	                base.appendChild(next);
	                next = doc.createElement("y");
	                next.setAttribute("num",Integer.toString(j));
	                next.appendChild(doc.createTextNode( Integer.toString((int)p[j].getY()) ));
	                base.appendChild(next);
	            }
	            
	            base = doc.createElement("transforms");
	            thing.appendChild(base);
	            for(int j=0;j<numFrames;j++) {
	                next = doc.createElement("t");
	                next.setAttribute("frame",Integer.toString(j));
	                double [] values = new double[6];
	                things.get(i).getAbs(j).getMatrix(values);
	                next.setAttribute("v",(things.get(i).getVisibility(j)==true?"true":"false"));
	                next.setAttribute("m00",Double.toString(values[0]));
	                next.setAttribute("m10",Double.toString(values[1]));
	                next.setAttribute("m01",Double.toString(values[2]));
	                next.setAttribute("m11",Double.toString(values[3]));
	                next.setAttribute("m02",Double.toString(values[4]));
	                next.setAttribute("m12",Double.toString(values[5]));
	                
	                base.appendChild(next);
	            }
	            
	        }
	        
	        
	        // print the animation to the file
	        TransformerFactory transformerFactory = TransformerFactory.newInstance();
	        Transformer transformer = transformerFactory.newTransformer();
	        DOMSource source = new DOMSource(doc);
	        StreamResult result = new StreamResult(f);
	        transformer.transform(source,result);
	        
	    } catch (ParserConfigurationException e) {
	        e.printStackTrace();
	    } catch (TransformerException e) {
	        e.printStackTrace();
	    }
	}
	
	// open a dialog to save the file
	public void save(Container c) {
	    if(choose.showSaveDialog(c) == JFileChooser.APPROVE_OPTION) {
	        File f = choose.getSelectedFile();
	        if (f.exists()) { // if it exists
	            int val = JOptionPane.showConfirmDialog( // does user want to overwrite?
	                          null,
	                          "'" + f.getName() + 
	                          "' already exists\n do you want to replace it?",
	                          "File Exists",
	                          JOptionPane.YES_NO_OPTION
	                      );
	            if (val == JOptionPane.YES_OPTION) { // overwrite the file
	                f.delete();
	                goodSave(f);
	            } else { // let user select a new name
	                save(c);
	            }
	        } else { // save the new file
	            goodSave(f);
	        }
	    }
	}
	
	public void setColour(Color in) {
        if(in!=null)drawColour = in;
    }
	
}
