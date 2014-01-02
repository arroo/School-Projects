package model;

import java.util.*;
import java.awt.geom.*;

public class Thing {
    ArrayList<Integer> xs;
    ArrayList<Integer> ys;
    ArrayList<Point2D> points;
    ArrayList<AffineTransform> sequential; // for each frame, step through the list to produce desired results
    ArrayList<AffineTransform> absolute; // for each frame, perform just this transformation
    ArrayList<Boolean> visible;
    
    public Thing(int x, int y, int n, int tot){
        xs = new ArrayList<Integer>();
        ys = new ArrayList<Integer>();
        points = new ArrayList<Point2D>();
        sequential = new ArrayList<AffineTransform>(tot);
        absolute = new ArrayList<AffineTransform>(tot);
        for(int i=0;i<tot;i++) {
            sequential.add(new AffineTransform());
            absolute.add(new AffineTransform());
        }
        visible = new ArrayList<Boolean>(tot);
        assert(n<=tot);
        for(int i=0;i<tot;i++)
            visible.add(new Boolean((i<n?false:true)));
        
    }
    public void add(int x, int y){
        points.add(new Point2D.Double(x,y));
        xs.add(x);
        ys.add(y);
    }
    public void log(){
        //System.out.println("in log");
        for(int i=0;i<visible.size();i++){
            System.out.println(
                "i:" + i +
                " v:" + visible.get(i) +
                " s:" + sequential.get(i) +
                " a:" + absolute.get(i) 
            );
        }
    }
    public void addFrame(int n){
        sequential.add(n,new AffineTransform());
        absolute.add(n,(AffineTransform)absolute.get(n).clone());
        visible.add(n,new Boolean(visible.get(n).booleanValue()));
    }
    
    public boolean touches(Point2D in, int n) {
        if(visible.get(n).booleanValue()==false) return false;
        Rectangle2D r = new Rectangle2D.Double(in.getX()-4,in.getY()-4,8.0,8.0);
        Point2D [] linepoints = getPoints(n);
        if(linepoints.length == 1) return r.contains(linepoints[0]);
        Line2D [] lines = new Line2D[linepoints.length-1];
        for (int i=0;i<lines.length;i++)
            lines[i] = new Line2D.Double(linepoints[i],linepoints[i+1]);
        
        for(Line2D value : lines) {
            if(r.intersectsLine(value))
                return true;
        }
        return false;
    }
    public boolean touches(int x, int y, int n){
        return touches(new Point2D.Double(x,y), n);
    }
    public boolean touches(double x, double y, int n){
        return touches(new Point2D.Double(x,y), n);
    }
    
    public Point2D [] getPoints(int n){
        Point2D [] returnPoints = new Point2D[getN()];
        Point2D [] basePoints = points.toArray(new Point2D[points.size()]);
        absolute.get(n).transform(basePoints,0,returnPoints,0,getN());
        return returnPoints;
    }
    public int getN(){
        return xs.size();
    }
    public int [] getXs(){
        int [] newYs = new int[getN()];
        int i = 0;
        for(Object value : xs.toArray()){
            Integer val = (Integer)value;
            newYs[i++] = val.intValue();
        }
        return newYs;
    }
    public int [] getYs(){
        int [] newYs = new int[getN()];
        int i = 0;
        for(Object value : ys.toArray()){
            Integer val = (Integer)value;
            newYs[i++] = val.intValue();
        }
        return newYs;
    }
    public boolean getVisibility(int n){
        if(n<visible.size())
            return visible.get(n).booleanValue();
        return false;
    }
    public void setVisibility(int n, boolean val){
        for (int i=n;i<visible.size();i++)
            visible.set(i,new Boolean(val));
    }
    
    public void move(int x, int y, int n){
        //System.out.println("s:"+sequential.size()+" a:"+absolute.size());
        for(int i=n+1;i<sequential.size();i++)
            sequential.get(i).setToIdentity();
        sequential.get(n).translate(x,y);
        AffineTransform abs = absolute.get(n);
        abs.setToIdentity();
        int i = 0;
        for(Object trans : sequential.toArray()){
            if (i>n) break;
            abs.concatenate((AffineTransform)trans);
            i++;
        }
        for(i=n;i<absolute.size();i++)
            absolute.get(i).setTransform(abs);
    }
    public AffineTransform getAbs(int n){
        AffineTransform at = new AffineTransform(absolute.get(n));
        return at;
    }
    
}
