# Premise:
"If you're not careful and you noclip out of reality in the wrong areas, you'll end up in the Backrooms, where it's nothing but the stink of old moist carpet, the madness of mono-yellow, the endless background noise of fluorescent lights at maximum hum-buzz, and approximately six hundred million square miles of randomly segmented empty rooms to be trapped in... <br>
God save you if you hear something wandering around nearby, because it sure as hell has heard you" - original post

Use your wits and whatever you find to survive and escape the backrooms. Good luck!

# File structure:
(Starting at program's "root" directory (x64/release/); these are duplicated to the project directory for debugging.

## /Text/
This directory holds all text displayed in the game for easy translation/localization. Files are named based on where they appear. Text is displayed line-by-line as it appears in these files, with no automatic wrap or special formatting unless noted otherwise. Files will be read and text updated once per frame to allow for semi-live editing. Characters beyond standard 7-bit ASCII are not currently included in the font graphics.

### menu text
Menu text files will contain one line for each piece of text in the menu (excluding live values read from program state).

### other text
other text displays until end of file is reached (may exceed visible screen area for long files).

## /Tiles/
This directory holds both tilemaps and tilesheets for the game.

### tilemaps:
Standard .txt files, using header matching that exported from PyxelEdit (shown below), followed immediately by a block of tile ID's separated by commas. The game uses the `tileswide [number of tiles]`, `tileshigh [number of tiles]`, and `layer 0` lines to parse the file, so it may crash if they are misplaced or cannot be located. <br>
_Header:_ <br>

    tileswide 16
    tileshigh 14
    tilewidth 16
    tileheight 16

    layer 0

### tilesheets:
Standard .png files, arranged in a grid 16 tiles wide. Tiles are each 16 tiles square unless otherwise noted.

Font is 8 pixels wide by 16 pixels high.
