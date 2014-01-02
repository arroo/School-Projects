import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import view.*;
//import infomat.view.menu.ToolbarView;

public class Sketch {

    public static void main(String[] args) {
        final int fps = 60;
        int startingSeconds = 5;
        final JFrame frame = new JFrame("Sketch");
        Container content = frame.getContentPane();
        content.setLayout(new BorderLayout());
        JPanel panel = new JPanel();
        final JLabel logLabel = new JLabel();
        final JSlider frameSlider = new JSlider(1,fps * startingSeconds-1,1);
        final Timer timex = new Timer(1000/fps, new ActionListener(){
		    public void actionPerformed(ActionEvent e){
		        frameSlider.setValue(frameSlider.getValue()+1);
		        
		        if(frameSlider.getValue()>=frameSlider.getMaximum())
		            ((Timer)e.getSource()).stop();
		    }
		});
		final JColorChooser chooser = new JColorChooser(Color.black);
        final view.Window window = new view.Window(fps, startingSeconds, 800,400, timex, frameSlider);
		JButton drawButton = new JButton("Draw");
		JButton eraseButton = new JButton("Erase");
		JButton selectButton = new JButton("Select");
		JButton playButton = new JButton("|>");
		JButton pauseButton = new JButton("||");
		JButton resetButton = new JButton("Reset");
		final JButton colourButton = new JButton("Colour");
		frameSlider.addKeyListener(new KeyListener(){
		    Timer t = new Timer(1000/fps,new ActionListener() {
	            public void actionPerformed(ActionEvent e) {
	                window.add();
	            }
	        });
		    public void keyTyped(KeyEvent e){}
		    public void keyPressed(KeyEvent e){
		        if (e.getKeyCode()==e.VK_CONTROL)
		            t.start();
		    }
		    public void keyReleased(KeyEvent e){
		        if (e.getKeyCode()==e.VK_CONTROL)
		            t.stop();
		    }
		});
		frameSlider.addMouseListener(new MouseAdapter(){
		    public void mousePressed(MouseEvent e){
		        timex.stop();
		    }
		});
		resetButton.addMouseListener(new MouseAdapter() {
		    public void mousePressed(MouseEvent e){
		        timex.stop();
		        frameSlider.setValue(frameSlider.getMinimum());
		    }
		});
        drawButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				window.nowDrawing();
		        colourButton.setEnabled(true);
			}
		});
		eraseButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				window.nowErasing();
		        colourButton.setEnabled(false);
			}
		});
		selectButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				window.nowSelecting();
		        colourButton.setEnabled(false);
			}
		});
		frameSlider.addChangeListener(new ChangeListener(){
		    public void stateChanged(ChangeEvent e) {
		        JSlider sl = (JSlider)e.getSource();
		        window.pickFrame(sl.getValue());
		        logLabel.setText(Integer.toString(frameSlider.getValue()));
		    }
		});
		playButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				window.nowPlaying();
			}
		});
		pauseButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				window.nowPausing();
			}
		});
		colourButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			    timex.stop();
				window.setColour(JColorChooser.showDialog(chooser,"Pick a Colour", Color.black));
			}
		});
		JButton logButton = new JButton("Log");
		
		logButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			    timex.stop();
			    System.out.println("Start Log");
				window.log();
				System.out.println("End Log");
			}
		});
		JToolBar toolbar = new JToolBar();
		toolbar.setFloatable(false);
		toolbar.setRollover(true);
		JPanel panel2 = new JPanel();
		panel.add(logLabel);
		panel.add(logButton);
		panel.add(pauseButton);
		panel.add(playButton);
		panel.add(frameSlider);
		panel.add(resetButton);
		toolbar.add(colourButton);
		toolbar.add(drawButton);
		toolbar.add(eraseButton);
		toolbar.add(selectButton);
		
		panel2.add(toolbar);
		
		JMenuBar menuBar = new JMenuBar();
		JMenu menu = new JMenu("File");
        JMenuItem menuItem = new JMenuItem("Save", KeyEvent.VK_S);
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Save?");
        menuItem.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e) {
			    window.save(frame);
			}
        });
        menu.add(menuItem);
        menuItem = new JMenuItem("Open", KeyEvent.VK_O);
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Open?");
        menuItem.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e) {
			    window.load(frame);
			}
        });
        menu.add(menuItem);
        menu.addSeparator();
        menuItem = new JMenuItem("Exit", KeyEvent.VK_X);
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X, ActionEvent.ALT_MASK));
        menuItem.getAccessibleContext().setAccessibleDescription("Exit?");
        menuItem.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e) {
			    System.exit(0);
			}
        });
        menu.add(menuItem);
		menu.setMnemonic(KeyEvent.VK_F);
        menu.getAccessibleContext().setAccessibleDescription("File Stuff");
        menuBar.add(menu);
		frame.setJMenuBar(menuBar);
		
		
		content.add(panel2,BorderLayout.NORTH);
		content.add(window, BorderLayout.CENTER);
		content.add(panel, BorderLayout.SOUTH);
		frame.setSize(800, 400);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setVisible(true);
        return;
    }
}
