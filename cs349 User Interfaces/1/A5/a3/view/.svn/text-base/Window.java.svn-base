package view;

import java.lang.Math;
import javax.swing.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import model.*;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
 
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;


public class Window extends JComponent {
    public static final int DRAW = 1;
    public static final int ERASE = 2;
    public static final int SELECT = 3;
    public static final int MOVE = 4;
    int mode;
	Image image;
	boolean inside;
	boolean selecting;
	Boolean moving;
	Rectangle2D selectBox;
	Thing selectThing;
	Point2D selectPoint;
	ArrayList<Thing> things;
	ArrayList<Color> colours;
	Color drawColour;
	ArrayList<Integer> selectedThings;
	Graphics2D graphics2D;
	int currentX, currentY, oldX, oldY;
	Thing newthing;
    javax.swing.Timer t;
    JSlider slide;
    int currentFrame;
    int numFrames;
    int fps;
    int wX,wY;
    JFileChooser choose;
    
    public void setColour(Color in){
        if(in!=null)drawColour = in;
        //System.out.println(""+in);
    }
    
    
	public Window(int f, int sec, final int windowX, final int windowY, final javax.swing.Timer timer, final JSlider sl){
	    slide = sl;
	    wX=windowX;
	    wY=windowY;
	    drawColour = new Color(0);
	    t = timer;
	    inside = false;
	    moving = new Boolean(false);
	    selecting = false;
	    numFrames = f*sec;
	    fps = f;
	    choose = new JFileChooser();
	    currentFrame = 0;
	    things = new ArrayList<Thing>();
	    colours = new ArrayList<Color>();
	    nowDrawing();
		setDoubleBuffered(true);
		addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent e) {
			    t.stop();
				oldX = e.getX();
				oldY = e.getY();
				switch (mode) {
				  case DRAW:
				    newthing = new Thing(oldX,oldY,currentFrame,numFrames);
				    colours.add(new Color(
				    drawColour.getRed(),
				    drawColour.getGreen(),
				    drawColour.getBlue()));
				    things.add(newthing);
				    newthing.add(oldX,oldY);
				    break;
				  case ERASE:
				    for(int i=0;i<things.size();i++){
				        if(things.get(i).touches(oldX,oldY,currentFrame))
				            things.get(i).setVisibility(currentFrame, false);
				    }
				    break;
				  case SELECT:
				    selectThing = new Thing(oldX,oldY,currentFrame,numFrames);
				    selectBox = null;
				    break;
				  case MOVE:
				    assert(selectBox!=null);
				    if(selectBox.contains(oldX,oldY)==false) {
				        mode = SELECT;
				        mousePressed(e);
				    } else {
				        selectPoint = new Point2D.Double(oldX,oldY);
				        moving = new Boolean(true);
				        t.start();
				    }
				    break;
				}
				repaint();
			}
			
			public void mouseReleased(MouseEvent e){
			    switch(mode) {
			      case SELECT:
			        selectedThings = new ArrayList<Integer>();
			        Polygon p = new Polygon(selectThing.getXs(),selectThing.getYs(),selectThing.getN());
			        double minX = windowX,minY=windowY,maxX = 0,maxY = 0;
			        
			        for(int i=0;i<things.size();i++) {
			            double tnX = windowX,tnY=windowY,txX = 0,txY = 0;
			            if (!things.get(i).getVisibility(currentFrame)) continue;
			            boolean containsAll = true;
			            Point2D [] testPoints = things.get(i).getPoints(currentFrame);
			            for(int j=0;j<testPoints.length;j++) {
			                tnX = Math.min(tnX,testPoints[j].getX());
			                tnY = Math.min(tnY,testPoints[j].getY());
			                txX = Math.max(txX,testPoints[j].getX());
			                txY = Math.max(txY,testPoints[j].getY());
			                if(p.contains(testPoints[j]) == false) {
			                    containsAll = false;
			                    break;
			                }
			            }
			            if (containsAll==true) {
			                minX=Math.min(minX,tnX);
			                minY=Math.min(minY,tnY);
			                maxX=Math.max(maxX,txX);
			                maxY=Math.max(maxY,txY);
			                selectedThings.add(i);
			            }
			        }
			        
			        if (!selectedThings.isEmpty()){
			            selectBox = new Rectangle2D.Double(minX,minY,maxX-minX,maxY-minY).getBounds2D();
			            mode = MOVE;
			        } else {
			            selectBox = null;
			        }
			        selectThing = null;
			        break;
			      case MOVE:
			        t.stop();
			        //mode = SELECT;
			        moving = new Boolean(false);
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
		        currentX = e.getX();
		        currentY = e.getY();
		        if (mode==ERASE)repaint();
		        oldX = currentX;
		        oldY = currentY;
		    }
		    
			public void mouseDragged(MouseEvent e) {
				currentX = e.getX();
				currentY = e.getY();
				if (graphics2D != null)
				    switch(mode) {
				      case DRAW:
				        newthing.add(currentX,currentY);
				        break;
				      case ERASE:
				        for(int i=0;i<things.size();i++){
				            if(things.get(i).touches(oldX,oldY,currentFrame))
				                things.get(i).setVisibility(currentFrame, false);
				        }
				        break;
				      case SELECT:
				        if (selectThing==null) // should never happen but does and not enough time to find out
				                               // why since this seems to make it stable
				            selectThing = new Thing(currentX,currentY,currentFrame, currentFrame+1);
				        selectThing.add(currentX,currentY);
				        break;
				      case MOVE:
				        assert(selectBox!=null);
				        selectBox.setRect(selectBox.getX()-oldX+currentX,selectBox.getY()-oldY+currentY,selectBox.getWidth(),selectBox.getHeight());
				        for(int i=0;selectedThings!=null&&i<selectedThings.size();i++){
				            things.get(selectedThings.get(i)).move(currentX-oldX,currentY-oldY,currentFrame);
				        }
				        break;
			    }
			    
				repaint();
				oldX = currentX;
				oldY = currentY;
			}
		});
	}
	
	private void now(){
	    t.stop();
	    selectThing = null;
	    selectBox = null;
	    mode = (mode==MOVE?SELECT:mode);
	    repaint();
	}
	
    public void nowDrawing(){
        mode = DRAW;
        now();
    }
    public void nowErasing(){
        mode = ERASE;
        now();
    }
    public void nowSelecting(){
        mode = SELECT;
        now();
    }
    public void nowPlaying(){
        now();
        t.start();
    }
    public void nowPausing(){
        now();
    }
    
    public void add() {
        slide.setMaximum(slide.getMaximum()+1);
        for(int i=0;i<things.size();i++)
            things.get(i).addFrame(currentFrame);
    }
    
	public void paintComponent(Graphics g) {
		if (image == null) {
			image = createImage(getSize().width, getSize().height);
			graphics2D = (Graphics2D) image.getGraphics();
			graphics2D.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
					RenderingHints.VALUE_ANTIALIAS_ON);
		}
		clear();
		for(int i = 0;i<things.size();i++){
		    //System.out.println("thing:"+i+ " visible:"+things.get(i).getVisibility(currentFrame));
		    if(things.get(i).getVisibility(currentFrame)) {
    		    //draw that bitch
    		    Point2D [] drawPoints = things.get(i).getPoints(currentFrame);
    		    int [] xs = new int[drawPoints.length];
    		    int [] ys = new int[drawPoints.length];
    		    for(int j=0;j<drawPoints.length;j++) {
    		        xs[j] = (int)Math.round(drawPoints[j].getX());
    		        ys[j] = (int)Math.round(drawPoints[j].getY());
    		    }
    		    graphics2D.setPaint(colours.get(i));
    		    graphics2D.drawPolyline(xs,ys,drawPoints.length);
    		}
		}
		graphics2D.setPaint(Color.black);
		if (mode == ERASE && inside)
            graphics2D.drawRect(currentX-4,currentY-4,8,8);
		if (mode == SELECT && selectThing != null)
		    graphics2D.drawPolyline(selectThing.getXs(),selectThing.getYs(),selectThing.getN());
		if(selectBox!=null)
		    graphics2D.draw(selectBox);
		g.drawImage(image, 0, 0, null);
	}

	public void clear() {
		graphics2D.setPaint(Color.white);
		graphics2D.fillRect(0, 0, getSize().width, getSize().height);
		graphics2D.setPaint(Color.black);
	}
	public void log(){
	    for(int i=0;i<things.size();i++){
	        System.out.println("Start Log: "+i);
	        things.get(i).log();
	    }
	}
	public void pickFrame(int i){
	    if (moving.booleanValue()==false&&mode==MOVE) {
	        selectBox = null;
	        selectedThings = null;
	        mode = SELECT;
	    } else if (mode==MOVE&&selectedThings!=null&&selectedThings.size()>0) {
	        for(int j=0;j<selectedThings.size();j++){
	            things.get(selectedThings.get(j)).move(0,0,currentFrame);
	            if(things.get(selectedThings.get(j)).getVisibility(i)==false ) // moving and invisible
	                things.get(selectedThings.get(j)).setVisibility(i,true);
	        }
	    }
	    if(i<numFrames)
    	    currentFrame = i;
    	repaint();
	}
	
	public void goodSave(File f){
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
	        
	        for(int i=0;i<things.size();i++){
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
	    /*try {
	        f.createNewFile();
	        BufferedWriter out = new BufferedWriter(new FileWriter(f), 32768);
	        StringBuilder s = new StringBuilder("<animation>\n");
	        s.append("<fps>60</fps>\n<frames>"+numFrames+"</frames>\n");
	        for(int i=0;i<things.size();i++){
	            s.append("<thing>\n");
	            s.append("<colour>\n<red>"+colours.get(i).getRed()+"</red><green>"+colours.get(i).getGreen()+"</green><blue>"+colours.get(i).getBlue()+"</blue>");
	            s.append("</colour>\n<base>\n");
	            Point2D [] p = things.get(i).getPoints(0);
	            for(int j=0;j<p.length;j++){
	                s.append("<x>"+p[j].getX()+"</x><y>"+p[j].getY()+"</y>\n");
	            }
	            s.append("</base>\n<transforms>\n");
	            for(int j=0;j<numFrames;j++) {
	                double [] values = new double[7];
	                things.get(i).getAbs(j).getMatrix(values);
	                s.append("<t m00=\""+values[0]+"\" m10=\""+values[1]+"\" m01=\""+values[2]+"\" m11=\""+values[3]+"\" m02=\""+values[4]+"\" m12=\""+values[0]+"></t>\n");
	            }
	            s.append("</transforms>\n<visibility>\n");
	            for(int j=0;j<numFrames;j++) {
	                s.append("<v>"+things.get(i).getVisibility(j)+"</v>\n");
	            }
	            s.append("</visibility>\n</thing>\n");
	        }	        
	        s.append("</animation>");
	        out.write(s.toString());
	        out.close();
	    } catch (IOException e) {}
	    catch (SecurityException e) {}*/
	}
	
	public void save(Container c){
	 
	    if(choose.showSaveDialog(c)==JFileChooser.APPROVE_OPTION){
	        File f = choose.getSelectedFile();
	        if(f.exists()){
	            //System.out.println("file exists");
	            int val = JOptionPane.showConfirmDialog(null,"'"+f.getName()+"' already exists\n do you want to replace it?","File Exists",JOptionPane.YES_NO_OPTION);
	            if(val==JOptionPane.YES_OPTION){
	                f.delete();
	                goodSave(f);
	            } else {
	                save(c);
	            }
	        } else {
	            goodSave(f);
	        } 
	        //f.close();
	    }
	}
	public void load(Container c){
	    
	    if(choose.showOpenDialog(c)==JFileChooser.APPROVE_OPTION){
	        
	    }
	}
}
