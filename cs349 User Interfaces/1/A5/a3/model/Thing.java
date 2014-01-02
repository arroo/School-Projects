package model;

import java.util.*;
import java.awt.geom.*;

/*
    This is a model for an object that can be drawn 
    as a series of line segments that can be erased 
    and transformed

*/

public class Thing {
    ArrayList<Integer> xs;                 // the x-values of the points that make up the object
    ArrayList<Integer> ys;                 // the x-values of the points that make up the object
    ArrayList<Point2D> points;             // a points representation that make up the object
    
    ArrayList<AffineTransform> sequential; // for each frame, step through the 
                                           // list to produce desired results
    ArrayList<AffineTransform> absolute;   // for each frame, perform just
                                           // this transformation
                                           
    ArrayList<Boolean> visible;            // an array of the visibility of the object
                                           // throughout the duration of the frames
    
    // Thing constructor
    public Thing(int x, int y, int n, int tot) {
        xs = new ArrayList<Integer>();
        ys = new ArrayList<Integer>();
        points = new ArrayList<Point2D>();
        sequential = new ArrayList<AffineTransform>(tot);
        absolute = new ArrayList<AffineTransform>(tot);
        for(int i = 0; i < tot; i++) {
            sequential.add(new AffineTransform());
            absolute.add(new AffineTransform());
        }
        visible = new ArrayList<Boolean>(tot);
        assert(n <= tot);
        for (int i = 0; i < tot; i++) // each Thing will be invisible at
                                      // first then appear during the frame
                                      // it was created on
            visible.add(new Boolean((i < n ? false : true)));
        
    }
    
    // function for adding more points to the Thing
    public void add(int x, int y){
        points.add(new Point2D.Double(x, y));
        xs.add(x);
        ys.add(y);
    }
    
    // function for adding a single frame to the animation
    public void addFrame(int n){
        sequential.add(n, new AffineTransform());
        absolute.add(n, (AffineTransform)absolute.get(n).clone());
        visible.add(n, new Boolean(visible.get(n).booleanValue()));
    }
    
    // function for determining if during a single frame a point
    // of the Thing is contained within a box around the user's cursor
    public boolean touches(Point2D in, int n) {
        if (visible.get(n).booleanValue() == false)
            return false; // during this frame the Thing is invisible
        
        Rectangle2D r = new Rectangle2D.Double(  // set-sized box around the cursor
                            in.getX() - 4,
                            in.getY() - 4,
                            8.0,
                            8.0
                        );
        Point2D [] linepoints = getPoints(n);
        
        if(linepoints.length == 1) // only 1 point is easy to check
            return r.contains(linepoints[0]);
            
        Line2D [] lines = new Line2D[linepoints.length - 1];
        for (int i=0;i<lines.length;i++) // recreate the lines of the Thing from the points
            lines[i] = new Line2D.Double(linepoints[i], linepoints[i + 1]);
        
        for (Line2D value : lines) { // check if any of the lines intersect the box
            if(r.intersectsLine(value)) // they do
                return true;
        }
        return false; // they don't by default
    }
    public boolean touches(int x, int y, int n) {
        return touches(new Point2D.Double(x,y), n);
    }
    public boolean touches(double x, double y, int n) {
        return touches(new Point2D.Double(x,y), n);
    }
    
    // function for returning all of the points in the Thing
    public Point2D [] getPoints(int n) {
        Point2D [] returnPoints = new Point2D[getN()];
        Point2D [] basePoints = points.toArray(new Point2D[points.size()]);
        absolute.get(n).transform(basePoints,0,returnPoints,0,getN());
        return returnPoints;
    }
    
    // function for finding the number of points in the Thing
    public int getN(){
        return xs.size();
    }
    
    // function for returning the x-values of the Thing in array form
    public int [] getXs(){
        int [] newXs = new int[getN()];
        int i = 0;
        for (Object value : xs.toArray()) {
            Integer val = (Integer)value;
            newXs[i++] = val.intValue();
        }
        return newXs;
    }
    
    // function for returning the y-values of the Thing in array form
    public int [] getYs() {
        int [] newYs = new int[getN()];
        int i = 0;
        for (Object value : ys.toArray()) {
            Integer val = (Integer)value;
            newYs[i++] = val.intValue();
        }
        return newYs;
    }
    
    // function for returning the visibility of the Thing during a certain frame
    public boolean getVisibility(int n) {
        if (n < visible.size())
            return visible.get(n).booleanValue();
        return false;
    }
    
    // function for setting the visiblity of the Thing to a value and
    // preserving it for all future frames
    public void setVisibility(int n, boolean val) {
        for (int i = n; i < visible.size(); i++)
            visible.set(i, new Boolean(val));
    }
    
    // function for moving the Thing by x, y on frame n
    public void move(int x, int y, int n) {
        
        for (int i = n + 1; i < sequential.size(); i++) // assume this frame is indicative
            sequential.get(i).setToIdentity();          // of all following frame

        sequential.get(n).translate(x,y); // translate this spot
        
        AffineTransform abs = absolute.get(n);
        abs.setToIdentity();
        int i = 0;
        for (Object trans : sequential.toArray()) { // recalculate the absolute 
            if (i > n) break;                       // AffineTransform for this frame
            abs.concatenate((AffineTransform)trans);
            i++;
        }
        for (i = n; i < absolute.size(); i++)  // set the absolute AffineTransforms for the
            absolute.get(i).setTransform(abs); // rest of the frames to the same as this
    }
    
    // function for returning the absolute AffineTransform of the Thing
    // for a specific frame
    // useful for animation
    public AffineTransform getAbs(int n){
        AffineTransform at = new AffineTransform(absolute.get(n));
        return at;
    }
    
}
