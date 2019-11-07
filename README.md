# Most - Preprocessing - Extraction

This repository contains a set of tools for the creation of spatio temporal data from raw images. It is being developed at IEETA (Institute of Electronics and Informatics Engineering of Aveiro) and DETI (Departament of Electronic, Telecommunications and Informatics) from the University of Aveiro (www.ua.pt) and is partially funded by National Funds through the FCT (Foundation for Science and Technology) in the context of projects UID/CEC/00127/2013 and POCI-01-0145-FEDER-032636.

## Available applications
- Extraction of data from images
    - **segmenter** - extracts contours from images data using manual input;
    - **auto_segmenter** - extracts contours automatically from images or videos by using a configuration file of objects of interest;
    - **hsv** - allows for interactive visualization of HSV color spaces to create filters for auto_segmenter.
    - **cell_extraction** - extract information from the 2D+time datasets available at http://celltrackingchallenge.net/
- Extraction of data from videos
  - **frame_extractor** - extracts specific frames selected manually or a given number of frames with equidistant sampling;
  - **auto_segmenter** - extracts contours automatically from images or videos by using a configuration file of objects of interest;
- Post processing
  - **draw_wkt** - draws any given polygon in WKT format on top of an image; can be used to verify quality of extracted data
  - **simplifier** - the segmentation extracts full contours. This tools allows for interactive or automated polygon simplification using different methods
  - **warp** - performs a perspective warp on a polygon. Can be used to project areas segmented on perspective images/videos into an orthonormal (map) perspective.

## Installation and use
Installation and instructions are available at the project Wiki pages - https://github.com/most-ieeta/preprocessing_extraction/wiki.
