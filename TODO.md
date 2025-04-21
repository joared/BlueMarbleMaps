Rendering:
	- Implement efficient redrawing similar to QtGraphicsItem (probably analogous to PresentationObject). PresentationObject should have a bounds() method.
		- PresentationObjects can be cached, and call its respective visualizer() to redraw
		- PresentationObjects/Features could have change flags similar to QQuickItem::ItemChange: 
			- added to view (use this attribute to visualize fade in animations)
		- Difference between QQuickItem and Feature/PresentationObject	
			Visualization
				- QQuickItem has parent and children. An object or feature could be represented by a QQuickItem parent/child relationship
				- Features are the objects, however, a Feature can result in multiple PresentationObjects (kinda children?)
			EventHandling
				- Each QQuickItem handles events itself. If not handled, it is propagated to the parent
				- Event handling of features/PresentationObjects are handled by a "global" eventhandler (or tools), that handles all objects generically
	- Concept of scene and view
	- Qt Quick SceneGraph (QtQuick)
		https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html
		https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph-renderer.html
	- QQuickitem vs QGraphicsItem

Coordinate system:

Event handling:
	- Each DataSet can have an associated EventHandler. When mouse events occur, hittests are performed. 
	- When a presentation object is hit, the EventHandler is notified, and retrieves the event. When the event handler is active (focus?), all events are forwarded. This is done using the Id::dataSetId() from the feature the presentation object represents.
	- If the dataset has no associated event handler, or if the event handler didn't handle the event, events are forwarded to the active tool. This gives flexibility and possibility to introduce GUI elements.
	- NOTE: Hittesting has to be performed with different pointer radius in this case. Better would be to handle GUI elements in a separate, explicit chain with pointer radius 0. If no GUI element is hit, the above happens with a specified pointer radius.

Selection of presentation objects (subparts of feature):
	- It would be beneficial to be able to hover/select a subpart of a feature, i.e. a specific presentation object. This would make it possible to e.g. add hover/selection visualization for a specific node on a polygon.
	- Possible solution: Layers has default, hover and selection visualizers that are correctly applied when the "whole" feature is selected as usual. Visualizers can have one hover/selection visualizer (not necessarily of the same kind?). When the application calls Map.hover(PresentationObject pObj) or Map.select(PresentationObject pObj), the visualizer that generated the presentation object gets notified that itself should keep track of the selected feature and apply its hover/select visualizer during the next update (if they exist). When features are attached during the next update, it will instead attach the feature (which it keeps track of) to its appropriate hover/selection visualizer. It would be easy for example for node visualization, define one default node (symbol) visualizer, another selection visualizer, but at the same time provide the selection visualizer to the default visualizer, such that the same visualizer is applied when selecting the presentation object or the whole feature itself. At some point, a deselection have to occur. There should be a deselect for PresentationObject explicitly, but whenever a deselectAll, call is made, the PresentationObjects are deselected as well as features.

Effieciency and optimization:
	- Use const FeaturePtr& instead of copying
	- Add performance monitor

Safe programming using pointers:
	- Features "owns" its geometry. It should not be possible to instantiate a Feature with a pointer to someone elses geometry.

Reflection:
	- How to parse containers (vector, map, etc)
