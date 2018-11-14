/*A device interface object contains:
  the device object
  a <div> tag
    a <canvas> tag
    <input> tags relating to sending commands to the device
  and methods for updating the canvas and validating device JSON files
*/
testVariable = 123456;
function deviceInterface() //this begins the deviceInterface constructor
{
  this.deviceObj; //the object representing the device
  this.divTag; //the enclosing <div> tag
  this.canvasTag; //the <canvas> used for drawing
  this.c_width = 500;
  this.c_height = 100;
  this.ctx; //the context of canvasTag

  //make tags
  this.divTag = document.createElement("div");
  this.canvasTag = document.createElement("canvas");
  this.canvasTag.width = this.c_width;
  this.canvasTag.height = this.c_height;
  this.canvasTag.innerHTML = "No Canvas :(";
  this.ctx = this.canvasTag.getContext("2d");

  //place canvasTag into divTag
  this.divTag.appendChild(this.canvasTag);

  this.updateObj = function (obj)
  {
    //if obj is a string, test for json validity
    if(typeof obj == "string")
    {
      try //try to convert obj to object
      {
        this.deviceObj = JSON.parse(obj);
      }catch(e)
      {
        //the only error JSON.parse() can throw is SyntaxError
        console.err(e);
        console.err("Bad JSON. Setting deviceInterface to empty device");
        this.deviceObj = {"device":"EMPTY", "outputs":[{"Name":"OUT_1","MIN_VALUE":0,
			     "MAX_VALUE":0,"Value":0}]}
      }
    }
    else if (typeof obj == "object") {
      this.deviceObj = obj;
    }
  }

  this.drawDevice = function ()
  {
    //this was copy-pasted from original html page.
    for (var i = 0; i < this.deviceObj.outputs.length; i++) {
   	/*
   	Make two rectangles for output display; left is solid, right has no in-fill
   	widths should be proportional to max-min and value
   	*/
   	var indicator_width = 100;
   	var indicator_height = 15;
   	var vert_spacing = 5;
   	var temp_obj = this.deviceObj.outputs[i];
   	var prop = temp_obj.Value / (temp_obj.MAX_VALUE - temp_obj.MIN_VALUE);
   	console.log("prop = " + prop);
   	//left and right width are for the indicator
   	var left_width = indicator_width * prop;
   	var right_width = indicator_width - left_width;
   	if (prop <= 1.0)
   		this.ctx.fillStyle = "#0000FF";
   	else
   		this.ctx.fillStyle = "#FF0000";
   	this.ctx.strokeStyle = "#0000FF";
   	this.ctx.fillRect(0, i * (indicator_height + vert_spacing), left_width, indicator_height); //solid rectangle
   	this.ctx.fillStyle = "#000000";
   	this.ctx.strokeRect(left_width, i * (indicator_height + vert_spacing) + 1, right_width, indicator_height - 2); //blank rectangle is two pixels smaller
   	//text
   	this.ctx.textBaseline = "top";
   	this.ctx.font = "normal " + indicator_height + "px Arial";
   	this.ctx.fillText(temp_obj.Name + ":\t" + temp_obj.Value, indicator_width + 10, i * (indicator_height + vert_spacing));
   	}
  }

}
