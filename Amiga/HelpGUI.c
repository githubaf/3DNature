/* HelpGUI.c
** World Construction Set Help for Settings Editing module.
*/

#include "GUIDefines.h"
#include "WCS.h"
#include "GUIExtras.h"

static void Help_ES_Window(APTR item) // used locally only -> static, AF 26.7.2021
{
 short i, gadtype = 0;
 static const char *GadType[] = {
  "Button", "String", "Integer String", "Float String", "Cycle"
 };

 static const char *GadNameStr[] = {
  "Frame Save Path",
  "Line Save Path",
  "Background Image Path",
  "Z Buffer Path",
  "Color Map Path",
  "Background File",
  "Z Buffer File",
  "Temporary Field Path",
  "Frame Save Name",
  "Temporary Field Name",
  "Line Save Name",
  "Forest Model Path"
 };

 static const char *GadNameIntStr[] = {
  "Start Frame",
  "Maximum Frames",
  "Step Frames",
  "Render Segments",
  "Image Width",
  "Image Height",
  "Vertical Overscan",
  "Fractal Depth",
  "Grid Size",
  "Surface Elevation 1",
  "Surface Elevation 2",
  "Surface Elevation 3",
  "Surface Elevation 4",
  "Sky Dither Range",
  "Blur Effect",
  "Scaled Width",
  "Scaled Height",
  "Vector Segments",
  "Look Ahead Frames"
 };

 static const char *GadNameFloatStr[] = {
  "Pixel Aspect",
  "Banking Factor",
  "Vector Offset",
  "Zenith Altitude",
  "Tree Height Factor",
  "Alt Z Buf Origin Lat",
  "Alt Z Buf Origin Lon",
  "Max Z Blur Offset",
  "Ecosystem Gradient",
  "Snow Gradient",
  "Global Reference Lat"
 };

 static const char *GadNameCycle[] = {
  "Render RGB",
  "Render Screen",
  "Render Data",
  "Global Gradients",
  "Save Format",
  "Concatenate",
  "Camera Path",
  "Focus Path",
  "Bank Turns",
  "Render Vectors",
  "Vector Haze",
  "Fixed Fractals",
  "Pre-Map",
  "Color Maps",
  "Random Borders",
  "Color Map Trees",
  "Color Match",
  "Forest Model",
  "Statistics",
  "Surface Grid",
  "Fixed Horizon",
  "Alt Z Buf Origin",
  "Cloud Shadows",
  "Ecosystem Flattening",
  "Look Ahead",
  "Background Image",
  "Z Buffer Pre-Load",
  "Blur Operator",
  "Z Buffered Blur",
  "Scale Image",
  "Topos as Surfaces",
  "Tree Displacement",
  " ",
  "Forest Model Match",
  "Export Z Buffer",
  "Z Buffer format",
  "Field Render"
 };

 static const char *GadMesgStr[] = {
  "Device and directory path to which rendered images and animation frames\
 will be saved.If the device becomes filled during a rendering session you\
 will be prompted to supply a new path.",

  "Device and directory path to which rendered vectors will be saved\
 (see \338Render \338Vectors\0332 cycle gadget).",

  "Device and directory path from which background images will be pre-loaded\
 (see \338Background\0332 cycle gadget).",

  "Device and directory path from which Z Buffer files will be pre-loaded\
 (see \338Z \338Buffer\0332 cycle gadget).",

  "Device and directory path from which color maps will be loaded during\
 rendering (see \338Color \338Maps\0332 cycle gadget).\n\
If a number is included in this name it must match the frame number.\n\
If there is no number the same color maps will be used for all frames.\n\
Frame numbers must be at the end of the directory name.",

  "File name for background images (see \338Background\0332 cycle gadget).\n\
If a number is included in the file name it must match the frame number.\n\
If there is no number the same image will be used for all frames.\n\
Frame numbers may be anywhere in the name, not necessarily at the end.\n\
If there is more than one number in the name the last one must be the frame.",

  "File name for Z buffer files (see \338Z \338Buffer\0332 cycle gadget).\n\
If a number is included in the file name it must match the frame number.\n\
If there is no number the same Z buffer will be used for all frames.\n\
Frame numbers may be anywhere in the name, not necessarily at the end.\n\
If there is more than one number in the name the last one must be the frame.",

  "Device and directory path for temporary storage of field rendered images\
 until both fields are rendered (see \338Field \338Rendering\0332 cycle gadget).",

  "File base name for final rendered images and animation frames\
 (see \338Save \338Format\0332 cycle gadget).\nFrame numbers will be appended.",

  "File base name for temporary field rendered images and animation frames\
 (see \338Field \338Rendering\0332 cycle gadget).\nFrame numbers will be appended.",

  "File base name for rendered vectors (see \338Render \338Vectors\0332 cycle\
 gadget).\nFrame numbers will be appended.",


  "Device and directory path from which forest models will be loaded during\
 rendering (see \338Ecosystem \338Editor\0332 window).\n\
If a number is included in this name it must match the frame number.\n\
If there is no number the same forest models will be used for all frames.\n\
Frame numbers must be at the end of the directory name.\n\
There may be a different model for each ecosystem."
 };


 static const char *GadMesgIntStr[] = {
  "Frame number to begin rendering.\n\
Animation frames need not be rendered consecutively.",

  "Number of frames to render in a single rendering session.",

  "Frame increment for rendering.\n\
Frames need not be rendered consecutively.",

  "Number of segments or bands to subdivide the image into for rendering\
 within limited memory.\nLarge print resolution images will often require\
 multiple segments.\nAny number of segments can be used as long as the\
 height of the image is evenly divisible by the number of segments.\
 Otherwise a few pixel rows may be lost at the bottom.",

  "Width in pixels of the desired output image. The limit is 32,767.",

  "Height in pixels of the desired output image. The limit is 32,767.",

  "Temporary pixel rows to add at bottom of image to capture the base of any\
 trees below the lower edge of images and segments whose tops may fall within\
 the final image dimensions.\nVertical Overscan should increase as foreground\
 trees become larger.\nA minimum value of 20 should be used but might need to\
 be as much as 200 for very close trees in very high resolution images.\n\
The value becomes extremely important in multi-segment rendered images.\n\
Having the value too large is preferable to too small though it will cost some\
 memory and additional processing time the larger it is.",

  "Level of polygon subdivision to add apparent detail to images.\n\
Increasing this increases detail and processing time.\n\
The maximum level supported is 9.",

  "For surfaces, the number of data cells between rendered grid lines\
 (see \338Surface \338Grid\0332 cycle gadget).",

  "For surfaces, the elevation in meters at which the first surface color\
 is applied (see \338Color \338Editor\0332 window) in a hypsometric shading\
 scheme involving four elevations and three color gradients (four colors).\n\
Also the lower limit of shading gradation in Map View for drawing topos and\
 surfaces when the \338Map \338Style\0332 cycle gadget is set to \"Surface\".\n\
Surface Elevation 1 should be the lowest of the four elevations.\n\
Elevations lower than Surface Elevation 1 will be rendered in the first\
 surface color.",

  "For surfaces, the elevation in meters at which the second surface color\
 is applied (see \338Color \338Editor\0332 window) in a hypsometric shading\
 scheme involving four elevations and three color gradients (four colors).\n\
Also the second lowest elevation used in Map View for drawing topos and surfaces\
 when the \338Map \338Style\0332 cycle gadget is set to \"Surface\".\n\
Surface Elevation 2 should be the second lowest of the four elevations.",

  "For surfaces, the elevation in meters at which the third surface color\
 is applied (see \338Color \338Editor\0332 window) in a hypsometric shading\
 scheme involving four elevations and three color gradients (four colors).\n\
Also the second highest elevation used in Map View for drawing topos and surfaces\
 when the \338Map \338Style\0332 cycle gadget is set to \"Surface\".\n\
Surface Elevation 3 should be the second highest of the four elevations.",

  "For surfaces, the elevation in meters at which the fourth surface color\
 is applied (see \338Color \338Editor\0332 window) in a hypsometric shading\
 scheme involving four elevations and three color gradients (four colors).\n\
Also the upper limit of shading gradation in Map View for drawing topos and\
 surfaces when the \338Map \338Style\0332 cycle gadget is set to \"Surface\".\n\
Surface Elevation 4 should be the highest of the four elevations.\n\
Elevations higher than Surface Elevation 4 will be rendered in the fourth\
 surface color.",

  "Number of color points variation allowed when dithering sky colors.\n\
For instance, if the computed red, green and blue values are 200, 210, 240,\
 and the Sky Dither Range is 20 (a good default value), the final color\
 values could range from 180-220, 190-230, 220-255.\n\
WCS will ensure that color values never fall outside the 0-255 range.",

  "Blur can be applied to images and frames after final rendering (including\
 vectors). This could be used to create a focussing or diffusion effect\
 although the blur factor will have to be adjusted manually for each frame.\n\
increasing this value will increase blurring.",

  "Images can be scaled to a different final size than the one at which they\
 are rendered (see \338Scale \338Image\0332 cycle gadget). This will provide\
 some additional antialiasing if rendered size is larger than the scaled\
 size.\nThis value controls the width of  the final image.\n\
\0333This option is not yet implemented!",

  "Images can be scaled to a different final size than the one at which they\
 are rendered (see \338Scale \338Image\0332 cycle gadget). This will provide\
 some additional antialiasing if rendered size is larger than the scaled\
 size.\nThis value controls the height of the final image.\n\
\333This option is not yet implemented!",

  "The number of vector line segments that will be rendered for a vector object\
 (beginning at the vertex corresponding to the frame count) if the \"Class\"\
 field of the object is set to \"Segment V\" or \"Illum Seg\" (see \338Database\
 \338Editor\0332 window)."
 };

 static const char *GadMesgFloatStr[] = {
  "Pixel aspect is the denominator of a ratio that can be adjusted to\
 compensate for differing display media.\nA value of 0.0 is usually used\
 for print media including color copiers and slides.\n\
Depending on the format and output device, various aspect\
 ratios may be appropriate for video. Try 1.2 for starters. To create more\
 vertical stretch decrease the pixel aspect.",

  "When using either a vector or key framed motion path turn banking can be\
 automated (see \338Bank \338Turns\0332 cycle gadget). This value controls the\
 steepness of banking as well as the direction.\nNormal values would range from\
 0.0 to 5.0.\nOptionally, banking can be controlled using key frames (see\
 \338Motion \338Editor\0332 window). Key Frames will produce more stable\
 results if the camera path is less than smooth (see \338Bank \338Turns\0332\
 cycle gadget for instructions on creating nice smooth paths).\nThe Banking Factor\
 is used for \"Create Bank Keys\" in the Motion Editor and in \"LightWave Motion\
 Path Export\" as well.",

  "Vector objects are Z Buffered into a rendered image. Since trees and\
 topographic irregularities may partially or fully obscure vectors this\
 offset value is provided to trick WCS into thinking that the vectors are\
 actually closer than they really are and allowing them to be rendered.\n\
The units are in miles. A value of 0.04 has worked well for the developers in\
 most situations. If you are using vectors with large segment lengths relative\
 to the size of DEM polygons you might need to increase this value.",

  "The horizon line can be made to follow camera motion (see \338Fixed\
 \338Horizon\0332 cycle gadget). The zenith altitude controls the steepness\
 of the color gradient between horizon color and zenith color (see \338Color\
 \338Editor\0332 window).\nThe value is in miles.",

  "All trees can be adjusted in height by this value. A doubling of this value\
 will double the height of all trees.\nTypical working values are from 3.0 to\
 12.0.\nYou may want to increase this value if you are using large amounts of\
 vertical exaggeration. For some reason that appears to the eye more correct.\n\
Tree species' heights may be adjusted individually in the Ecosystem Editor.",

  "If for some obscure reason you should want to use a different origin than\
 the camera position for computing Z factors (used in Z buffer operations)\
 you may set an alternate origin latitude here (see \338Alt \338Z\
 \338Reference\0332 cycle gadget).",

  "If for some obscure reason you should want to use a different origin than\
 the camera position for computing Z factors (used in Z buffer operations)\
 you may set an alternate origin longitude here (see \338Alt \338Z\
 \338Reference\0332 cycle gadget).",

  "Post-rendering blur can be limited to pixels representing proximal areas.\
 For instance if you want to blur a surface to simulate Gouraud shading but\
 still want distinct ridge lines, this is the technique for you.\n\
The value is in miles of Z offset between one pixel and the next.\nPixels\
less than this distance apart will be blurred together, those farther apart\
 will not be affected (see \338Blur\0332 cycle gadget).",

  "A gradient can be applied to ecosystems based on latitude to simulate\
cooler environments toward the poles (see \338Global \338Gradients\0332\
 cycle gadget). This value controls the equivalence of latitude degrees\
 to elevation in meters. Typical values are close to 100.0 meters/degree.\n\
This value applies to all ecosystems except water and snow (see \338Snow\
 \338m/°\0332 string gadget).",

  "A gradient can be applied to the snow ecosystem based on latitude to\
 simulate cooler environments toward the poles (see \338Global\
 \338Gradients\0332 cycle gadget). This value controls the equivalence of\
 latitude degrees to elevation in meters. Typical values are close to 100.0\
 meters/degree.\nThis value applies to only the snow ecosystem (see \338Eco\
 \338m/°\0332 string gadget).",

  "A gradient can be applied to ecosystems based on latitude to\
 simulate cooler environments toward the poles (see \338Eco\
 \338m/°\0332 string gadget). The gradients are applied relative to the\
 latitude of the ecosystems defined in the \338Ecosystem \338Editor\0332.\
That reference latitude is defined in this value.\nSince gradients are applied\
 symmetrical to the equator the value here should be positive although in\
 practice WCS will take the absolute value of it anyway.",

  "The number of frames for the camera to look ahead of its own position\
 along the camera path (see \338Look \338Ahead\0332 cycle gadget).\n\
Minimum value is one frame ahead. The rougher the camera path the farther\
 you will need to \"look ahead\" to avoid causing stomach distress to animation\
 viewers. Looking farther ahead may be desirable anyway to lead the eye into\
 turns."
 };

 static const char *GadMesgCycle[] = {
  "Determines whether 24 bit color data is rendered. This is the only rendered\
 bitmap data that is saved.",
  "Determines whether 4 bit images are rendered to the screen. These are for\
 user information only and are not saved.",
  "Determines whether or not diagnostic information is saved and presented to\
 the user upon completion of a rendering. This is not available for\
 multi-segmented images or during multiple animation frame rendering sessions.\n\
The diagnostics are of the same form as in Interactive View (see \338Motion\
 \338Editor\0332 window) and can be used as a background for digitizing\
 vector objects. They can also be used for determining Z buffer values of\
 specific topographic features.",
  "When enabled, global gradients will apply an elevation gradient to\
 ecosystem and snow line values based on polygon latitude relative to the\
 reference latitude.\nThis is generally used to represent cooler temperatures\
 toward the poles. The gradients are applied symmetrically to the equator.",
  "Rendered images can be saved in one of three formats: IFF, Raw component and\
 Interleaved component.",
  "Multi-segmented images can either be concatenated into one final output\
 file or left as separate horizontal segments.\nThe virtue of not concatenating\
 is the ability on small systems to work with rather large images in paint\
 or image processing programs. Component files can later be concatenated with\
 the AmigaDOS \"Join\" command.",
  "If you wish the camera motion path to follow a specific vector object, set\
 this to true. Also set the desired vector's \"Class\" field in the database\
 to \"Cam Path\".",
  "If you wish the focus motion path to follow a specific vector object, set\
 this to true. Also set the desired vector's \"Class\" field in the database\
 to \"Foc Path\".",
  "This will allow automatic banking of turns for either key framed paths or\
 follow-vector paths. Obviously you must have a defined path one way or the\
 other for this to work.\nSet the amount of banking in the string to the\n\
 right.\nAutomatic banking seems to work best and smoothest\
 with key framed paths because the splining is smoother than with vector paths\
 created with \"Interpolate path\" in map.\nFor the ultimate in sophistication\
 try laying out a rough outline of your path in map. Then do a \"Path\
 Interpolation\" to get about one tenth of the final desired frames set.\
 Convert this vector to motion key frames at an interval of 10 frames per\
 vector segment. Go into \"Interactive View\" to set camera altitudes.",
  "Vector objects can be treated in several ways during rendering. They may\
 be output to the bitmap, output to a file or disabled completely.\nIf you\
 get a warning in Interactive View about vectors being turned off in the\
 Settings Editor, this is the place to fix the problem.",
  "Haze and shading can be added to vector objects or not as you desire.",
  "For animations you will almost always want this enabled to keep trees and\
 other formations rooted in place. Of course there is a price: It is somewhat\
 slower to render. Usually for still images you will turn this off to gain the\
 speed advantage. Also for very slow moving animations where you are moving\
 perpendicular to the camera lens axis you may be able to disable\
 \"Fixed Fractals\".",
  "Once upon a time, a very long time ago, all WCS images were first rendered\
 at a fractal level of 0 with no trees. This allowed an increase in speed as\
 the more detailed layer was added by giving a Z buffer value to discriminate\
 hidden polygons and trees against. Pre-mapping is no longer necessary and in\
 fact is faster without. The only reason you may want to enable this is if\
 at very high vertical exaggerations you find some \"holes\" in the landscape.\
 We think this will not happen, but it's better to be safe and provide some\
 kind of a way to deal with it if it does.",
  "Color maps are used for a great many purposes in WCS. Before you can use\
 them they must be enabled here.\nThe gadgets below determine how color maps\
 will be used.\nTheir functions are basically to pass information to the\
 renderer about how you want certain areas to be drawn. Any area for which\
 no color map exists or the color components of the color map are 0,0,0 will\
 be rendered with ecosystems as normal. When a color is present it can be used\
 directly to tint the landscape or as an ecosystem index or as a combination.",
  "Randomized borders will soften the edges of color boundaries. This is\
 referring to the actual color boundaries not the color map or DEM boundaries.\
 It applies regardless of the purpose color maps serve.",
  "If color maps are being used to tint the landscape, trees can be drawn in\
 the tint color or not.\nWhen disabled there will be no trees drawn in color\
 mapped areas.",
  "Color maps can be used as an ecosystem index. Color component values are\
 set for specific ecosystems in the Ecosystem Editor. When these components\
 are found to match identically the components of the color map at a point, the\
 referenced ecosystem will be drawn at that point. It can be used to maintain\
 precise placement of trees, water, snow or to map out entire environments.\n\
If no color match is found for the color map components, then the color is\
 used to tint the landscape.\nMaximum slope constraints designated for a color-\
matched ecosystem will still apply.",
  "At one time WCS supported user-designed forest models which allowed very\
 fine control over species composition and stand structure (height\
 variability). At the present time this feature has gathered some moss and\
 needs to be re-worked. It should be back in action again soon and more\
 powerful than ever.",
  "Statistical terrain data (slopes, aspects & elevations) can be tabulated for\
 color mapped regions. The results are saved to an ascii file which can be\
 viewed in Map.\nOnly the areas for which a non-black color is present in the\
 color map will be tabulated in the statistics.",
  "For surfaces, a grid of latitude/longitude lines may be rendered either into\
 the bitmap or a vector file. Which one will depend on the Render Vectors cycle\
 gadget setting.",
  "The horizon may be fixed at the position determined in the Motion Editor or\
 allowed to float as the camera moves. For animations this cycle gadget should\
 probably always be disabled. The advantage for stills is that \"Fixed\
 Horizon\" allows more precise user control of the horizon position.",
  "If for some reason you should wish to reference the Z Buffer to some\
 point other than the camera, enable this and set the reference latitude and\
 longitude in the strings below.",
  "WCS regrettably does not yet support cloudy skies but we can give you cloud\
 shadows on the ground.\nTo use this first create some cloud maps. Like color\
 maps, there should be one for every DEM and at the same resolution (normally\
 301 x301). It should be a gray scale image (8 bit) saved in either IFF or\
 Raw component format. The file should have the same name as the corresponding\
 DEM object with \".cloud\" appended. The files should be stored in the same\
 directory as normal color maps. For the cloud shadows, a gray value of 128 is\
 neutral. Darker values darken the landscape proportionally, brighter values\
 lighten it.\nEven with no clouds in the sky this can be used to add\
 interesting highlights and mood to your images. Of course they can be\
 animated (see Color Map Directory string for more information).",
  "When flattening is applied through the Motion Editor parameters (Flattening\
 and Datum) ecosystem lines can be adjusted automatically to compensate.",
  "For animations the camera can be made to point along the motion path.\
 Set the frame string to the right for the number of frames in advance of the\
 camera to look.\nThe frame number must always be one or more.\n\
Look Ahead will override focus paths and focus path key frames.",
  "You may pre-load an image or series of images (for animations) over which\
 WCS will render. Background images might be used to provide some clouds or\
 in combination with Z Buffers to provide foreground details or animated\
 objects.\nImages must be IFF or Raw component format of the same dimensions\
 as the WCS render image size.\nThe background image will be antialiased into\
 WCS' rendering.",
  "A Z Buffer may be pre-loaded against which all WCS render values will be\
 compared.\nZ Buffer files may be in IFF format (we can supply the details)\
 or as a Width x Height four byte, floating point array. The size must match\
 your WCS render image size.\nZ Buffers may be animated and used in combination\
 with background images. Z Buffer arrays can be manipulated in the DEM\
 Converter in the Data Ops module if you wish to scale Z values.",
  "Final rendered images may have a blur applied to soften them although they\
 are automatically antialiased during rendering. This may reduce flickering\
 if it is a problem on NTSC or interlaced displays.",
  "Blurring may be limited to a distance offset between adjacent pixels.\n\
 This might be used for keeping ridges in focus while the details of the\
 terrain are softened.",
  "Image scaling can be applied after rendering. You may want to render larger\
 and scale down to achieve additional anti-aliasing.\nThis feature is not yet\
 functional.",
  "If you want to render all topo DEM's as hypsometrically shaded surfaces with\
 color gradients as defined in the Color Editor, enable this feature.\n\
Individual DEM's may be designated as surfaces in the \"Class\" field of the\
 database.",
  "If rendering aerial perspectives and you find an undesirable alignment of\
 trees, it may be due to imperfect random number generation or certain\
 topographic coincidences. In either event, enabling this Y offset feature\
 will attempt to break up those alignments.",
 "",
  "Forest models can be matched to the RGB components of a color map.\nThis\
 feature is not yet implemented.",
  "Set this if you wish to save the rendered Z Buffer for later use.",
  "Z buffers can be saved as 4 byte floating point arrays or as 8 bit gray\
 scale arrays (suitable for viewing in an image processing program that can\
 load Raw Gray images). The floating point format is necessary if you want\
 to import them for future WCS renderings. You can convert between formats in\the Data\
 Ops module DEM Converter.",
  "Field Rendering is used to smooth frame-frame motion. It improves the\
 appearance of even very slow moving animations.\nSadly it takes twice as long\
 to render since WCS must interpolate every other field and render it\
 separately.\nThis is a feature for the discriminating professional!"
 };

 
/* test for string */
 for (i=0; i<11; i++)
  {
  if (item == ES_Win->Str[i])
   {
   gadtype = 1;
   break;
   } /* if found match */
  } /* for i=0... */
/* test for integer string */
 if (! gadtype)
  {
  for (i=0; i<18; i++)
   {
   if (item == ES_Win->IntStr[i])
    {
    gadtype = 2;
    break;
    } /* if found match */
   } /* for i=0... */
  } /* if no match found yet */
/* test for float string */
 if (! gadtype)
  {
  for (i=0; i<11; i++)
   {
   if (item == ES_Win->FloatStr[i])
    {
    gadtype = 3;
    break;
    } /* if found match */
   } /* for i=0... */
  } /* if no match found yet */
/* test for cycle */
 if (! gadtype)
  {
  for (i=0; i<37; i++)
   {
   if (item == ES_Win->Cycle[i])
    {
    gadtype = 4;
    break;
    } /* if found match */
   } /* for i=0... */
  } /* if no match found yet */

 switch (gadtype)
  {
  case 0:
   {
   sprintf(str, "%s", GadType[0]);
   User_Message(str, "No help available at this time.", "OK", "o");
   break;
   } /*  */
  case 1:
   {
   sprintf(str, "%s: %s", GadNameStr[i], GadType[1]);
   User_Message(str, (char *)GadMesgStr[i], "OK", "o");
   break;
   } /*  */
  case 2:
   {
   sprintf(str, "%s: %s", GadNameIntStr[i], GadType[2]);
   User_Message(str, (char *)GadMesgIntStr[i], "OK", "o");
   break;
   } /*  */
  case 3:
   {
   sprintf(str, "%s: %s", GadNameFloatStr[i], GadType[3]);
   User_Message(str, (char *)GadMesgFloatStr[i], "OK", "o");
   break;
   } /*  */
  case 4:
   {
   sprintf(str, "%s: %s", GadNameCycle[i], GadType[4]);
   User_Message(str, (char *)GadMesgCycle[i], "OK", "o");
   break;
   } /*  */
  } /* switch */

} /* ES_Win_Help() */
