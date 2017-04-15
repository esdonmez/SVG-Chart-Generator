#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <math.h>
#define PI 3.14159265358979323846
/*Validation */
#define LIBXML_SCHEMAS_ENABLED
#include <libxml/xmlschemastypes.h>

static void createStruct(xmlNode * a_node);
char *strtok(char *s, const char *delim) ;
void parsePath (char *path);
void parseXML();
void validation();
double calculateX(double xCenter, double alpha, double radius);
double calculateY(double yCenter, double alpha, double radius);
void readCommandLine(int numberOfArg, char *ar[]);
void drawPieChart();
void drawLineChart();
void drawBarChart();
void printHelp();
int compare(const void * a,const void * b);
char* concat(char *s1,char *s2);


char * temp;
int isValidate = 0; // 0 ---> fails to validate   1 ---> validate
char pathForXML[20];
char pathForXSD[20];
char outputName[20];
char type[20];
char substring[2];

typedef struct Chartgendata
{
  char *charttitle;
}Chartgendata;

typedef struct Canvas
{
  char *length;
  char *width;
  char *backcolor;
}Canvas;

typedef struct Axis
{
  char *xName;
  char *yName;
  char *xForecolor;
  char *yForecolor;
}Axis;

typedef struct Xset
{
  char *monthArray[12];
}Xset;

typedef struct Yset
{
  char *unit;
  char *name;
  char *showvalue;
  char *fillcolor;
  char *salesArray[300];
}Yset;

Chartgendata chartgendata;
Canvas canvas;
Axis axis;
Xset months;
Yset sales;
Yset cityArray[300];

int numberOfCity = 0;
int numberOfMonth = 0;
int numberOfSales = 0;

int error = 0;

int main(int argc, char *argv[])
{
  /* Reading from commandline*/
  readCommandLine(argc, argv);
  if(error == 0)
  {

  /* XML Validation*/
    validation();
  /* XML Parse */
    parseXML();

    if (strcmp(type, "line") == 0)
    {
      drawLineChart();
    }
    else if (strcmp(type, "pie") == 0)
    {
      drawPieChart();
    }
    else
      drawBarChart();
  }

  return (0);
}

void validation()
{
  xmlDocPtr doc;
  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;
  char *XMLFileName = NULL;
  char *XSDFileName = NULL;
  XMLFileName = pathForXML;
  XSDFileName = pathForXSD;

  xmlLineNumbersDefault(1);

  ctxt = xmlSchemaNewParserCtxt(XSDFileName);

  xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
  schema = xmlSchemaParse(ctxt);
  xmlSchemaFreeParserCtxt(ctxt);

  doc = xmlReadFile(XMLFileName, NULL, 0);

  if (doc == NULL)
  {
    fprintf(stderr, "Could not parse %s\n", XMLFileName);
  }
  else
  {
    xmlSchemaValidCtxtPtr ctxt;
    int ret;

    ctxt = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctxt, doc);
    if (ret == 0)
    {
      isValidate = 1;
      printf("%s validates\n", XMLFileName);
    }
    else if (ret > 0)
    {
      printf("%s fails to validate\n", XMLFileName);
    }
    else
    {
      printf("%s validation generated an internal error\n", XMLFileName);
    }
    xmlSchemaFreeValidCtxt(ctxt);
    xmlFreeDoc(doc);
  }

// free the resource
  if(schema != NULL)
    xmlSchemaFree(schema);

  xmlSchemaCleanupTypes();
  xmlCleanupParser();
  xmlMemoryDump();

}

void parseXML()
{
  /*Parsing the XML File after passing validation*/
  if (isValidate == 1)
  {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    char *Filename = NULL;

    Filename = pathForXML;

    doc = xmlReadFile(Filename, NULL, 0);

    if (doc == NULL)
    {
      printf("error: could not parse file %s\n", Filename);
    }
    else
    {
      root_element = xmlDocGetRootElement(doc);

      createStruct(root_element);
      /*
       * free the document
       */
       xmlFreeDoc(doc);
     }
    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
     xmlCleanupParser();
   }
 }

 void readCommandLine(int argc, char *argv[])
 {
  int i;
  int strsize = 0;
  for (i=1; i<argc; i++)
  {
    strsize += strlen(argv[i]);
    if (argc > i+1)
      strsize++;
  }

  char *cmdSingleStr;
  cmdSingleStr = malloc(strsize);
  cmdSingleStr[0] = '\0';

  for (i=1; i<argc; i++)
  {
    strcat(cmdSingleStr, argv[i]);
    if (argc > i+1) strcat(cmdSingleStr, " ");
  }
  char *str ="-i xml -o program -v xsd -t chartgen";
  str = cmdSingleStr;

  int iCount = 0;
  int oCount = 0;
  int tCount = 0;
  int vCount = 0;

  if(argc == 2 && strcmp(argv[1], "-h") == 0)
  {
    printHelp();
    error = 1;
  }
  else if (argc != 9 || argc < 2)
  {
    printHelp();
    error = 1;
  }
  else if(argc == 9)
  {
    char * split;

    split = strtok (str,"-");

    while (split != NULL)
    {
      strncpy(substring, split, 1);
      substring[1] = '\0';

      if(strcmp(substring, "i") == 0)
      {
        strcpy(pathForXML, split);
        iCount++;
      }
      else if (strcmp(substring, "o") == 0)
      {
        strcpy(outputName, split);
        oCount++;
      }
      else if (strcmp(substring, "v") == 0)
      {
        strcpy(pathForXSD, split);
        vCount++;
      }
      else if (strcmp(substring, "t") == 0)
      {
        strcpy(type, split);
        tCount++;
      }

      split = strtok (NULL, "-");
    }

    if(iCount == 0 || oCount == 0 || vCount == 0 || tCount == 0)
    {
      printHelp();
      error = 1;
    }
    else
    {
      parsePath(pathForXML);
      parsePath(outputName);
      parsePath(pathForXSD);
      parsePath(type);
    }
  }
}

static void createStruct(xmlNode * a_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)  //XML_ELEMENT_NODE == 2
    {
      cityArray[numberOfCity].showvalue = (char*)malloc(sizeof(strlen(cur_node->children->content)+1));
      strcpy(cityArray[numberOfCity].showvalue, "no");

      if(strcmp(cur_node->name, "charttitle") == 0 && strcmp(cur_node->parent->name, "chartgendata") == 0)
      {
        chartgendata.charttitle = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(chartgendata.charttitle, cur_node->children->content);
      }
      if(strcmp(cur_node->name, "length") == 0 && strcmp(cur_node->parent->name, "canvas") == 0)
      {
        canvas.length = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(canvas.length, cur_node->children->content);
      }

      if(strcmp(cur_node->name, "width") == 0 && strcmp(cur_node->parent->name, "canvas") == 0)
      {
        canvas.width = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(canvas.width, cur_node->children->content);
      }

      if(strcmp(cur_node->name, "backcolor") == 0 && strcmp(cur_node->parent->name, "canvas") == 0)
      {
        canvas.backcolor = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(canvas.backcolor, "#");
        strcat(canvas.backcolor, cur_node->children->content);
      }

      if(strcmp(cur_node->name, "name") == 0 && strcmp(cur_node->parent->name, "Xaxis") == 0)
      {
        axis.xName = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(axis.xName, cur_node->children->content);
      }

      if(strcmp(cur_node->name, "name") == 0 && strcmp(cur_node->parent->name, "Yaxis") == 0)
      {
        axis.yName = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(axis.yName, cur_node->children->content);
      }

      if(strcmp(cur_node->name, "forecolor") == 0 && strcmp(cur_node->parent->name, "Yaxis") == 0)
      {
        axis.yForecolor = (char*)malloc(sizeof(strlen(cur_node->children->content) + 1));
        strcpy(axis.yForecolor, "#");
        strcat(axis.yForecolor, cur_node->children->content);
      }

      if(strcmp(cur_node->name, "xdata") == 0 && strcmp(cur_node->parent->name, "Xset") == 0)
      {
        months.monthArray[numberOfMonth] = (char*)calloc(sizeof(strlen(cur_node->children->content) + 1), 1);
        strcpy(months.monthArray[numberOfMonth], cur_node->children->content);
        numberOfMonth++;
      }

      if(strcmp(cur_node->name, "Yset") == 0)
      {
        numberOfSales = 0;
        xmlAttr* cur_attr = cur_node->properties;

        while(cur_attr)
        {
          xmlChar* value = xmlNodeListGetString(cur_node->doc, cur_attr->children, 1);


          if(strcmp(cur_attr->name, "unit") == 0)
          {
            cityArray[numberOfCity].unit = (char*)malloc(sizeof(strlen(cur_node->children->content)+1));
            strcpy(cityArray[numberOfCity].unit, value);
          }

          if(strcmp(cur_attr->name, "name") == 0)
          {
            cityArray[numberOfCity].name = (char*)malloc(sizeof(strlen(cur_node->children->content)+1));
            strcpy(cityArray[numberOfCity].name, value);
          }

          if(strcmp(cur_attr->name, "showvalue") == 0)
          {
            cityArray[numberOfCity].showvalue = (char*)malloc(sizeof(strlen(cur_node->children->content)+1));
            strcpy(cityArray[numberOfCity].showvalue, value);
          }

          if(strcmp(cur_attr->name, "fillcolor") == 0)
          {
            cityArray[numberOfCity].fillcolor = (char*)malloc(sizeof(strlen(cur_node->children->content)+1));
            strcpy(cityArray[numberOfCity].fillcolor, "#");
            strcat(cityArray[numberOfCity].fillcolor, value);
          }

          xmlFree(value);
          cur_attr = cur_attr->next;
        }
        numberOfCity++;
      }

      if(strcmp(cur_node->name, "ydata") == 0 && strcmp(cur_node->parent->name, "Yset") == 0)
      {
        cityArray[numberOfCity - 1].salesArray[numberOfSales] = (char*)calloc(sizeof(strlen(cur_node->children->content) + 1),1);
        strcpy(cityArray[numberOfCity - 1].salesArray[numberOfSales], cur_node->children->content);
        numberOfSales++;
      }
    }
    createStruct(cur_node->children);
  }
}

void parsePath(char *path)
{
  temp = strtok(path, " ");
  temp = strtok(NULL, " ");
  strcpy(path, temp);
}

char* concat(char *s1,char *s2){

  char *result = malloc(strlen(s1) + strlen(s2) + 1);
  strcpy(result,s1);
  strcat(result,s2);
  return result;

}

void drawLineChart()
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL ;/* node pointers */
    xmlDtdPtr dtd = NULL;       /* DTD pointer */
    char buff[256];
    char style[100]; // creating style attribute if svg polyline tag.
    int i, j;

    int spaceFromRight = 0;
    int spaceFromTop = 0;
    int x = 0; //Coordinate X
    int y = 0; //Coordinate Y
    char toStringX[6];
    char toStringY[6];

    char toStringX1[6];
    char toStringY1[6];
    char toStringX2[6];
    char toStringY2[6];

    spaceFromRight = atoi(canvas.width)  / 14;
    spaceFromTop   = atoi(canvas.length) / 14;

    int width = atoi(canvas.width);
    int length = atoi(canvas.length);
    /*
     * Creates a new document, a node and set it as a root node
     */
     doc = xmlNewDoc(BAD_CAST "1.1");
     root_node = xmlNewNode(NULL, BAD_CAST "svg");
     /*Setting of height, width and background color of svg charts.*/
     xmlDocSetRootElement(doc, root_node);
     xmlNewProp(root_node, BAD_CAST "height", BAD_CAST canvas.length);
     xmlNewProp(root_node, BAD_CAST "width", BAD_CAST canvas.width);

     node = xmlNewChild(root_node, NULL, BAD_CAST "rect", BAD_CAST NULL);
     xmlNewProp(node, BAD_CAST "height", BAD_CAST canvas.length);
     xmlNewProp(node, BAD_CAST "width", BAD_CAST canvas.width);
     xmlNewProp(node, BAD_CAST "fill", BAD_CAST canvas.backcolor);

     x = spaceFromRight * 7;
     y = spaceFromTop / 2 ;

     sprintf(toStringX, "%d" , x);
     sprintf(toStringY, "%d" , y);

     node = xmlNewChild(root_node,NULL, BAD_CAST "text",BAD_CAST chartgendata.charttitle); // Title
     xmlNewProp(node,BAD_CAST"x",BAD_CAST toStringX);
     xmlNewProp(node,BAD_CAST"y",BAD_CAST toStringY);
     xmlNewProp(node,BAD_CAST"text-anchor",BAD_CAST "middle");
     xmlNewProp(node,BAD_CAST"font-size",BAD_CAST toStringY);
     xmlNewProp(node,BAD_CAST"font-weight",BAD_CAST "bold");
     xmlNewProp(node,BAD_CAST"fill",BAD_CAST "#260A39");

     x = spaceFromRight * 13;
     y = spaceFromTop * 13;


     sprintf(toStringX, "%d" , x);
     sprintf(toStringY, "%d" , y);

     node = xmlNewChild(root_node,  NULL, BAD_CAST "text", BAD_CAST axis.yName); // Months
     xmlNewProp(node, BAD_CAST "x", BAD_CAST toStringX);
     xmlNewProp(node, BAD_CAST "y", BAD_CAST toStringY);
     x = spaceFromRight / 3;
     sprintf(toStringX, "%d" , x);

     xmlNewProp(node,BAD_CAST"font-size",BAD_CAST toStringX);
     xmlNewProp(node, BAD_CAST "fill", BAD_CAST "black");

     x = spaceFromRight;
     y = spaceFromTop * 7 / 8;

     sprintf(toStringX, "%d" , x);
     sprintf(toStringY, "%d" , y);

     node = xmlNewChild(root_node, NULL, BAD_CAST "text", BAD_CAST axis.xName); // Sales
     xmlNewProp(node, BAD_CAST "x", BAD_CAST toStringX);
     xmlNewProp(node, BAD_CAST "y", BAD_CAST toStringY);
     x = spaceFromRight / 3;
     sprintf(toStringX, "%d" , x);
     xmlNewProp(node,BAD_CAST"font-size",BAD_CAST toStringX);
     xmlNewProp(node, BAD_CAST "fill", BAD_CAST "black");

     int x1 = 0;
     int x2 = 0;
     int y1 = 0;
     int y2 = 0;

     x1 = spaceFromRight;
     x2 = spaceFromRight;
     y1 = spaceFromTop;
     y2 = spaceFromTop *13;

     sprintf(toStringX1, "%d" , x1);
     sprintf(toStringX2, "%d" , x2);
     sprintf(toStringY1, "%d" , y1);
     sprintf(toStringY2, "%d" , y2);

     node = xmlNewChild(root_node, NULL, BAD_CAST "line", NULL); // Vertical Line
     xmlNewProp(node, BAD_CAST "x1", BAD_CAST toStringX1);
     xmlNewProp(node, BAD_CAST "y1", BAD_CAST toStringY1);
     xmlNewProp(node, BAD_CAST "x2", BAD_CAST toStringX2);
     xmlNewProp(node, BAD_CAST "y2", BAD_CAST toStringY2);
     x = width / 100 / 2;
     sprintf(toStringX, "%d" , x);
     strcpy(style, "stroke:rgb(255,0,0);stroke-width: ");
     strcat(style, toStringX);
     xmlNewProp(node, BAD_CAST "style", BAD_CAST style);




     x1 = spaceFromRight * 13;
     x2 = spaceFromRight;
     y1 = spaceFromTop * 13;
     y2 = spaceFromTop * 13;

     sprintf(toStringX1, "%d" , x1);
     sprintf(toStringX2, "%d" , x2);
     sprintf(toStringY1, "%d" , y1);
     sprintf(toStringY2, "%d" , y2);

     node = xmlNewChild(root_node, NULL, BAD_CAST "line", NULL); // Horizontal Line
     xmlNewProp(node, BAD_CAST "x1", BAD_CAST toStringX1);
     xmlNewProp(node, BAD_CAST "y1", BAD_CAST toStringY1);
     xmlNewProp(node, BAD_CAST "x2", BAD_CAST toStringX2);
     xmlNewProp(node, BAD_CAST "y2", BAD_CAST toStringY2);
     x = width / 100 / 2;
     sprintf(toStringX, "%d" , x);
     strcpy(style, "stroke:rgb(255,0,0);stroke-width: ");
     strcat(style, toStringX);
     xmlNewProp(node, BAD_CAST "style", BAD_CAST style);




     /*
      * Sorting all of sales and defining max and min sales rates
     */
      int salesArray[numberOfCity*numberOfSales];
      int index=0;
      for(i=0; i<numberOfCity; i++)
      {
        for(j=0; j<numberOfSales;j++)
        {
          salesArray[index]=(int)atoi(cityArray[i].salesArray[j]);
          index++;
        }
      }

      int sizeofArray=0;

      sizeofArray=sizeof(salesArray)/sizeof(salesArray[0]);

      qsort(salesArray,sizeofArray,sizeof(int),compare);

      int max = salesArray[sizeofArray-1];
      int min = salesArray[0];

      int increaseAmount = (max - min) / numberOfSales;

      x = 3;
      y = spaceFromTop + spaceFromTop / 2;
      int fontSize = spaceFromTop / 3;
     char toStringFontSize[6]; // Font size of sales

     char toStringSales[6];
     int salesScale = max;
     int a;
     for (a = 0; a < numberOfSales; a++) // Writing sales on the left of the screen
     {

      sprintf(toStringX, "%d" , x);
      sprintf(toStringY, "%d" , y);
      sprintf(toStringSales, "%d" , salesScale);
      sprintf(toStringFontSize, "%d" , fontSize);

      node = xmlNewChild(root_node, NULL, BAD_CAST "text", toStringSales);
      xmlNewProp(node, BAD_CAST "x", BAD_CAST toStringX);
      xmlNewProp(node, BAD_CAST "y", BAD_CAST toStringY);
      xmlNewProp(node, BAD_CAST "font-size", BAD_CAST toStringFontSize);
      xmlNewProp(node, BAD_CAST "fill", BAD_CAST "blue");
        //xmlNewProp(node, BAD_CAST"font-weight", BAD_CAST "bold");

      salesScale = salesScale - increaseAmount;
      y = y + spaceFromTop * 12 / numberOfSales;
    }

    x1 = spaceFromRight * 13;
    x2 = spaceFromRight;
    y1 = spaceFromTop + spaceFromTop / 2;


     for (a = 0; a < numberOfSales; a++) // Printing dash arrays ( - - - - - )
     {

      sprintf(toStringX1, "%d" , x1);
      sprintf(toStringX2, "%d" , x2);
      sprintf(toStringY1, "%d" , y1);

      node = xmlNewChild(root_node, NULL, BAD_CAST "line", NULL);
      xmlNewProp(node, BAD_CAST "x1", BAD_CAST toStringX1);
      xmlNewProp(node, BAD_CAST "y1", BAD_CAST toStringY1);
      xmlNewProp(node, BAD_CAST "x2", BAD_CAST toStringX2);
      xmlNewProp(node, BAD_CAST "y2", BAD_CAST toStringY1);
      xmlNewProp(node, BAD_CAST "stroke", BAD_CAST "5184AF");
      xmlNewProp(node, BAD_CAST "stroke-width", BAD_CAST "1");
      xmlNewProp(node, BAD_CAST "stroke-dasharray", BAD_CAST "8, 2");
      xmlNewProp(node, BAD_CAST "style", BAD_CAST "stroke:rgb(255,0,0);stroke-width:0.3");

      y1 = y1 + spaceFromTop * 12 / numberOfSales;
    }


    x = spaceFromRight * 2;
    y = spaceFromTop * 13 + (spaceFromRight / 6);
    increaseAmount = spaceFromRight * 12 / numberOfMonth;

    node = xmlNewChild(root_node, NULL, BAD_CAST "text", NULL);
    xmlNodePtr tspanNode = NULL;

    int monthsFontSize = width / 60;
    char toStringMonthsSize[6];

     for(i = 0; i <numberOfMonth; i++){ // Printing months on the bottom of screen


      sprintf(toStringX, "%d" , x);
      sprintf(toStringY, "%d" , y);
      sprintf(toStringMonthsSize, "%d" , monthsFontSize);

      tspanNode = xmlNewChild(node, NULL, BAD_CAST "tspan", BAD_CAST months.monthArray[i]);
      xmlNewProp(tspanNode, BAD_CAST"x", BAD_CAST toStringX);
      xmlNewProp(tspanNode, BAD_CAST"y", BAD_CAST toStringY);
      xmlNewProp(tspanNode, BAD_CAST"writing-mode", BAD_CAST "tb");
      xmlNewProp(tspanNode, BAD_CAST"font-weight", BAD_CAST "bold");
      xmlNewProp(tspanNode, BAD_CAST "font-size", BAD_CAST toStringMonthsSize);

      x = x + increaseAmount;

    }


    int scale = salesArray[1] - salesArray[0];
    if (scale == 0){
      scale = 150;
    }

    double totalScale = (max - min) * scale / scale;


    index = 0;
    for(i=0; i<numberOfCity; i++)
    {
      for(j=0; j<numberOfSales;j++)
      {
        salesArray[index]=(int)atoi(cityArray[i].salesArray[j]);
        index++;
      }
    }

     /*
      * Calculating of x and y coordinates of circles and creating points for path.
      */

      for (j = 0; j < numberOfCity; j++)
      {
        char *path="";
        x = spaceFromRight * 2;
        y = spaceFromTop * 12;
        int showValueX = x;
        int showValueY = y;
        int r = width / 115;
        char toStringR[6];
        int valueSize = width / 50;
        char toStringValueSize[6];

        increaseAmount = spaceFromRight * 12 / numberOfMonth;
        for(i = 0; i <numberOfMonth; i++)
        {
          double valueOfSale = (int)atoi(cityArray[j].salesArray[i]);
          if((int)atoi(cityArray[j].salesArray[i]) == max){
            valueOfSale = spaceFromTop + spaceFromTop / 2;
          }
          else{
            valueOfSale = 12 * valueOfSale / max;
            valueOfSale = valueOfSale * spaceFromTop;
            valueOfSale = width - valueOfSale;
          }
          sprintf(toStringX, "%d" , x);
          sprintf(toStringY, "%.2f" , valueOfSale);
          path = concat(path, toStringX);
          path = concat(path, ",");
          path = concat(path, toStringY);
          path = concat(path, " ");
          sprintf(toStringR, "%d" , r);

          node = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);
          xmlNewProp(node, BAD_CAST "cx", BAD_CAST toStringX);
          xmlNewProp(node, BAD_CAST "cy", BAD_CAST toStringY);
          xmlNewProp(node, BAD_CAST "data-value", BAD_CAST "7.2");
          xmlNewProp(node, BAD_CAST "r", BAD_CAST toStringR);
          if (j == 0){
            xmlNewProp(node, BAD_CAST "fill", BAD_CAST "red");
          }
          else if (j == 1)
            xmlNewProp(node, BAD_CAST "fill", BAD_CAST "black");
          else if (j == 2)
            xmlNewProp(node, BAD_CAST "fill", BAD_CAST "purple");
          else
            xmlNewProp(node, BAD_CAST "fill", BAD_CAST "green");

          x = x + increaseAmount;

        }
        /* Creating line paths. */
        x = width / 140;
        sprintf(toStringX, "%d" , x);
        node = xmlNewChild(root_node, NULL, BAD_CAST "polyline", NULL);
        xmlNewProp(node, BAD_CAST "points", BAD_CAST path);
        xmlNewProp(node, BAD_CAST "stroke-width", BAD_CAST toStringX);
        strcpy(style, "fill:none;stroke: ");
        strcat(style, cityArray[j].fillcolor);
        xmlNewProp(node, BAD_CAST "style", BAD_CAST style);
      }
/*
 *   Writing values on the line (show value).
 */

 int legendX = spaceFromRight * 11;
 int legendY = spaceFromTop * 3;
 int legendSize = width/100;
 char toStringLegendX[6];
 char toStringLegendY[6];
 char toStringLegendSize[6];

 for (j = 0; j < numberOfCity; j++)
 {
  x = spaceFromRight * 2;
  y = spaceFromTop * 12;

  int showValueX = x;
  int showValueY = y;
  int valueSize = width / 50;
  char toStringValueSize[6];

  increaseAmount = spaceFromRight * 12 / numberOfMonth;
  for(i = 0; i <numberOfMonth; i++)
  {
    double valueOfSale = (int)atoi(cityArray[j].salesArray[i]);

    if((int)atoi(cityArray[j].salesArray[i]) == max)
    {
      valueOfSale = spaceFromTop + spaceFromTop / 2;
    }
    else
    {
      valueOfSale = 12 * valueOfSale / max;
      valueOfSale = valueOfSale * spaceFromTop;
      valueOfSale = width - valueOfSale;
    }

    sprintf(toStringX, "%d" , x);
    sprintf(toStringY, "%.2f" , valueOfSale);

    if(strcmp(cityArray[j].showvalue, "yes") == 0)
    {
      sprintf(toStringValueSize, "%d" , valueSize);
      sprintf(toStringX, "%d" , x + width / 40);
      showValueY = valueOfSale - length / 200;
      sprintf(toStringY, "%d" , showValueY);
      node = xmlNewChild(root_node,NULL, BAD_CAST "text",BAD_CAST cityArray[j].salesArray[i]);
      xmlNewProp(node,BAD_CAST"x",BAD_CAST toStringX);
      xmlNewProp(node,BAD_CAST"y",BAD_CAST toStringY);
      xmlNewProp(node,BAD_CAST"font-size",BAD_CAST toStringValueSize);
      xmlNewProp(node,BAD_CAST"text-anchor",BAD_CAST "middle");
    }

    x = x + increaseAmount;
  }

  sprintf(toStringLegendX, "%d" , legendX);
  sprintf(toStringLegendY, "%d" , legendY);
  sprintf(toStringLegendSize, "%d" , valueSize);

  node = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);
  xmlNewProp(node, BAD_CAST"x", BAD_CAST toStringLegendX);
  xmlNewProp(node, BAD_CAST"y", BAD_CAST toStringLegendY);
  xmlNewProp(node, BAD_CAST"width", BAD_CAST toStringLegendSize);
  xmlNewProp(node, BAD_CAST"height", BAD_CAST toStringLegendSize);
  xmlNewProp(node, BAD_CAST"fill", BAD_CAST cityArray[j].fillcolor);

  x = legendX + width / 30;
  y = legendY + width / 60;
  sprintf(toStringX, "%d" , x);
  sprintf(toStringY, "%d" , y);

  node = xmlNewChild(root_node,NULL, BAD_CAST "text",BAD_CAST cityArray[j].name);
  xmlNewProp(node,BAD_CAST"x",BAD_CAST toStringX);
  xmlNewProp(node,BAD_CAST"y",BAD_CAST toStringY);
  sprintf(toStringX, "%d" , width / 50);
  xmlNewProp(node,BAD_CAST"font-size",BAD_CAST toStringX);

  legendY = legendY - width / 30;
  //xmlNewProp(node, BAD_CAST"stroke", BAD_CAST "black");
}


    /*
     * Dumping document to stdio or file
     */
     xmlSaveFormatFileEnc(outputName, doc, "UTF-8", 1);


    /*free the document */
     xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
     xmlCleanupParser();

     xmlMemoryDump();

   }
   
   void drawPieChart()
   {
    xmlDocPtr doc = NULL;
    xmlNodePtr rootNode = NULL,node=NULL,node1=NULL;
    doc = xmlNewDoc(BAD_CAST"1.1");

    char colorArray[12][500] = {"#00ff00", "#ff9900", "#0000ff", "#ff0099", "#ff9999", "#009999", "#AA0000", "#AABB00", "#00CC55", "#44CCAA", "#220000", "#333333"};
    double alpha[numberOfMonth - 1];

    char style[80];
    double sumofSales = 0;
    double radius = 0;

    double canvaswidth = atof(canvas.width);
    double canvaslength = atof(canvas.length);

    double xHeader = (13*canvaswidth)/40;
    char _xHeader[200];

    sprintf(_xHeader, "%.2f", xHeader);

    double yHeader = (20*canvaslength)/400;
    char _yHeader[200];

    sprintf(_yHeader, "%.2f", yHeader);

    char fontsize[200];
    char _fontsize[20];

    strcpy(fontsize, "font-family: verdana, arial, sans-serif;font-size: ");
    sprintf(_fontsize, "%f", canvaslength/25);
    strcat(fontsize, _fontsize);
    strcat(fontsize, ";font-weight: bold;fill: #0099ff;stroke: none");

  // resized canvas
    canvaslength = canvaslength - yHeader/400;
    canvaslength = canvaslength/numberOfCity;

  //finding center coordinates
    double xCenter = canvaswidth/4;
    double yCenter = canvaslength/3 + yHeader + yHeader/5;
    double _yCenter = yCenter;
    double xLineTo = 0;
    double yLineTo = 0;

  if(canvaswidth/2 <= canvaslength) //finding radius
  {
    radius = (90*canvaswidth)/400;
  }
  else if (canvaswidth/2 > canvaslength)
  {
    radius = (canvaslength*123)/400;
  }

  rootNode = xmlNewNode(NULL, BAD_CAST"root");
  xmlDocSetRootElement(doc, rootNode);

  //calculating canvas size
  node = xmlNewChild(rootNode, NULL, BAD_CAST"svg", NULL);
  xmlNewProp(node, BAD_CAST"height", BAD_CAST canvas.length);
  xmlNewProp(node, BAD_CAST"width", BAD_CAST canvas.width);

  node1 = xmlNewChild(node, NULL, BAD_CAST"rect", NULL);
  xmlNewProp(node1, BAD_CAST"height", BAD_CAST canvas.length);
  xmlNewProp(node1, BAD_CAST"width", BAD_CAST canvas.width);
  xmlNewProp(node1, BAD_CAST"fill", BAD_CAST canvas.backcolor);

//printing general header
  node1 = xmlNewChild(node, NULL, BAD_CAST"text", BAD_CAST chartgendata.charttitle);
  xmlNewProp(node1, BAD_CAST"x", BAD_CAST _xHeader);
  xmlNewProp(node1, BAD_CAST"y", BAD_CAST _yHeader);
  xmlNewProp(node1, BAD_CAST"style", BAD_CAST fontsize);

  int i, j, x, z;
  int flag = 0;
  double yShowValue, xRectangle, xText, xShowValue , yText, yRectangle;
  double sale;
  char _path[200];
  char path[200];
  int skip = 0;

  for(z = 0; z < numberOfCity; z++)
  {
    sumofSales = 0;
    sale = 0;
    alpha[i] = 0;
    strcpy(path, "");

    //total sales
    for(i = 0; i < numberOfMonth; i++)
    {
      sale = atoi(cityArray[z].salesArray[i]);
      sumofSales = sumofSales + sale;
    }
    sale = 0;

    for(i = 0; i < numberOfMonth; i++)
    {
      sale = atoi(cityArray[z].salesArray[i]);

      if(i == 0)
      {
        alpha[i] = sale/sumofSales;
      }
      else if(i > 0)
      {
        alpha[i] = alpha[i-1] + sale/sumofSales;
      }

      if(sale/sumofSales > (0.5)) flag = 1;
      else if(sale/sumofSales <= (0.5)) flag = 0;

      strcpy(_path, "");
      strcpy(path,"");

      //Creating path for drawing pie chart
      if(i == 0)
      {
        strcpy(path,"M");
        sprintf(_path,"%.2f",xCenter);
        strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",yCenter); strcat(path, _path);
        strcat(path," L");
        sprintf(_path,"%.2f",(xCenter-radius)); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",yCenter); strcat(path, _path);
        strcat(path," A");
        sprintf(_path,"%.2f",radius); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",radius); strcat(path, _path);
        strcat(path," ");
        sprintf(_path,"%d",0); strcat(path, _path);
        strcat(path," ");
        sprintf(_path,"%d",flag); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%d",1); strcat(path, _path);
        strcat(path," ");
        sprintf(_path,"%.2f",calculateX(xCenter, alpha[i], radius)); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",calculateY(yCenter, alpha[i], radius)); strcat(path, _path);
        strcat(path," Z");
      }
      else if(i > 0)
      {
        strcpy(path,"M");
        sprintf(_path,"%.2f",xCenter); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",yCenter); strcat(path, _path);
        strcat(path," L");
        sprintf(_path,"%.2f",xLineTo); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",yLineTo); strcat(path, _path);
        strcat(path," A");
        sprintf(_path,"%.2f",radius); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",radius); strcat(path, _path);
        strcat(path," ");
        sprintf(_path,"%d",0); strcat(path, _path);
        strcat(path," ");
        sprintf(_path,"%d",flag); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%d",1); strcat(path, _path);
        strcat(path," ");
        sprintf(_path,"%.2f",calculateX(xCenter, alpha[i], radius)); strcat(path, _path);
        strcat(path,",");
        sprintf(_path,"%.2f",calculateY(yCenter, alpha[i], radius)); strcat(path, _path);
        strcat(path," Z");
      }

      xLineTo = calculateX(xCenter, alpha[i], radius);
      yLineTo = calculateY(yCenter, alpha[i], radius);

      strcpy(style, "stroke:black;stroke-width: 0.3;fill: ");
      strcat(style, colorArray[i]);

    //printing pie chart
      node1 = xmlNewChild(node, NULL, BAD_CAST"path", NULL);
      xmlNewProp(node1, BAD_CAST"d", BAD_CAST path);
      xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);

      char yCoordinate[15];
      char xCoordinate[15];
      char rectangleLength[15];
      char rectangleWidth[15];

      double h = canvaslength*22.219*9/(800*numberOfMonth);

    //finding coordinates
      if(z > 0)
      {
        if(i == 0)
        {
          yText = ((canvaslength*61)/400 - (23*canvaslength)/400) + canvaslength*141*skip/400 + yHeader*(z+1);
        }
        yRectangle = ((canvaslength*49)/400 + (i*23*canvaslength)/400) + canvaslength*141*skip/400 + yHeader*(z+1);
        yShowValue = ((canvaslength*61)/400 + (i*23*canvaslength)/400) + canvaslength*141*skip/400 + yHeader*(z+1);
        xRectangle = (canvaswidth*29)/40;
        xText = (canvaswidth*32)/40;
      }
      else if(z==0)
      {
        if(i == 0)
        {
          yText = ((canvaslength*61)/400 - (23*canvaslength)/400) + yHeader;
        }
        yRectangle = ((canvaslength*49)/400 + (i*23*canvaslength)/400) + yHeader;
        yShowValue = ((canvaslength*61)/400 + (i*23*canvaslength)/400) + yHeader;
        xRectangle = (canvaswidth*29)/40;
        xText = (canvaswidth*32)/40;
        xShowValue = (canvaswidth*25)/40;
      }

//printing show value
      if(strcmp(cityArray[z].showvalue, "yes") == 0)
      {
        sprintf(xCoordinate, "%f", xShowValue);
        sprintf(yCoordinate, "%f", yShowValue);
        sprintf(rectangleLength, "%f", h);
        sprintf(rectangleWidth, "%f", h*2);
        strcpy(style, "font-family:verdana, arial, sans-serif;fill: black;stroke: none;font-size: ");
        strcat(style, rectangleLength);

        node1 = xmlNewChild(node, NULL, BAD_CAST"text", BAD_CAST cityArray[z].salesArray[i]);
        xmlNewProp(node1, BAD_CAST"x", BAD_CAST xCoordinate);
        xmlNewProp(node1, BAD_CAST"y", BAD_CAST yCoordinate);
        xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);
      }

      sprintf(xCoordinate, "%f", xRectangle);
      sprintf(yCoordinate, "%f", yRectangle);
      sprintf(rectangleLength, "%f", h);
      sprintf(rectangleWidth, "%f", h*2);
      strcpy(style, "stroke: none;fill: ");
      strcat(style, colorArray[i]);

//printing color rectangles
      node1 = xmlNewChild(node, NULL, BAD_CAST"rect", NULL);
      xmlNewProp(node1, BAD_CAST"x", BAD_CAST xCoordinate);
      xmlNewProp(node1, BAD_CAST"y", BAD_CAST yCoordinate);
      xmlNewProp(node1, BAD_CAST"width", BAD_CAST rectangleWidth);
      xmlNewProp(node1, BAD_CAST"height", BAD_CAST rectangleLength);
      xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);

      strcpy(style, "font-family:verdana, arial, sans-serif;fill: black;stroke: none;font-size: ");
      strcat(style, rectangleLength);
      sprintf(yCoordinate, "%f", yShowValue);
      sprintf(xCoordinate, "%f", xText);

//printing values
      node1 = xmlNewChild(node, NULL, BAD_CAST"text", BAD_CAST months.monthArray[i]);
      xmlNewProp(node1, BAD_CAST"x", BAD_CAST xCoordinate);
      xmlNewProp(node1, BAD_CAST"y", BAD_CAST yCoordinate);
      xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);

      strcpy(style, "font-family:verdana, arial, sans-serif;fill: ");
      strcat(style, "black");
      strcat(style, "black;stroke: none;font-size: ");
      strcat(style, rectangleLength);

      sprintf(yCoordinate, "%f", yText);
      sprintf(xCoordinate, "%f", xText);

//printing value header
      node1 = xmlNewChild(node, NULL, BAD_CAST"text", BAD_CAST axis.xName);
      xmlNewProp(node1, BAD_CAST"x", BAD_CAST xCoordinate);
      xmlNewProp(node1, BAD_CAST"y", BAD_CAST yCoordinate);
      xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);

//printing show value header
      if(strcmp(cityArray[z].showvalue, "yes") == 0)
      {
        strcpy(style, "font-family:verdana, arial, sans-serif;fill:");
        strcat(style, axis.yForecolor);
        strcat(style, "black;stroke: none;font-size: ");
        strcat(style, rectangleLength);
        sprintf(yCoordinate, "%f", yText);
        sprintf(xCoordinate, "%f", xShowValue);

        node1 = xmlNewChild(node, NULL, BAD_CAST"text", BAD_CAST axis.yName);
        xmlNewProp(node1, BAD_CAST"x", BAD_CAST xCoordinate);
        xmlNewProp(node1, BAD_CAST"y", BAD_CAST yCoordinate);
        xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);
      }

//printing city names
      if(i == 3)
      {
        sprintf(rectangleLength, "%f", h*2);
        strcpy(style, "font-family:verdana, arial, sans-serif;fill:");
        strcat(style, cityArray[z].fillcolor);
        strcat(style, "black;stroke: none;font-size: ");
        strcat(style, rectangleLength);
        sprintf(yCoordinate, "%f", yShowValue);
        sprintf(xCoordinate, "%f", xCenter + radius + radius/3);

        node1 = xmlNewChild(node, NULL, BAD_CAST"text", BAD_CAST cityArray[z].name);
        xmlNewProp(node1, BAD_CAST"x", BAD_CAST xCoordinate);
        xmlNewProp(node1, BAD_CAST"y", BAD_CAST yCoordinate);
        xmlNewProp(node1, BAD_CAST"style", BAD_CAST style);
      }
    }

    //adding space between different charts
    yCenter = (z+2)*_yCenter + radius*(z+1);
    skip += 2;

    xmlSaveFormatFileEnc(outputName, doc, "UTF-8", 1);
  }
}

double calculateX(double xCenter, double alpha, double radius)
{
  double radian = alpha*2*PI;
  return xCenter - (cos(radian)) * radius;
}

double calculateY(double yCenter, double alpha, double radius)
{
  double radian = alpha*2*PI;
  return yCenter - (sin(radian)) * radius;
}


void drawBarChart()
{
  xmlDocPtr doc = NULL;
  xmlNodePtr nodeSVG = NULL, nodeRect = NULL, nodeTspan = NULL, nodeLine = NULL,nodeG = NULL, nodeText = NULL;
  doc = xmlNewDoc(BAD_CAST"1.1");

  nodeSVG = xmlNewNode(NULL, BAD_CAST "svg");
  xmlNewProp(nodeSVG, BAD_CAST"height", BAD_CAST canvas.length); 
  xmlNewProp(nodeSVG, BAD_CAST"width", BAD_CAST canvas.width);

  xmlDocSetRootElement(doc,nodeSVG);

  int barLength = (int)atoi(canvas.length) * 6 / 8 ;  
  int xPosition;
  int yPosition;
  char spaceX[10], sizeA[10],spaceY[10],_spaceX[10],strokeSize[10];
  yPosition = barLength /12;
  xPosition = (int)atoi(canvas.width) / 2;
  int size = barLength * 11 / 150;
  sprintf(sizeA, "%d", size);
  sprintf(spaceX, "%d", xPosition);
  sprintf(spaceY, "%d", yPosition);

  nodeRect = xmlNewChild(nodeSVG,NULL, BAD_CAST "rect",NULL);
  xmlNewProp(nodeRect,BAD_CAST"width",BAD_CAST canvas.length);
  xmlNewProp(nodeRect,BAD_CAST"height",BAD_CAST canvas.width);
  xmlNewProp(nodeRect,BAD_CAST"fill",BAD_CAST canvas.backcolor);
  xmlNewProp(nodeRect, BAD_CAST"stroke", BAD_CAST "black");

  nodeText = xmlNewChild(nodeSVG,NULL, BAD_CAST "text",BAD_CAST chartgendata.charttitle);
  xmlNewProp(nodeText,BAD_CAST"x",BAD_CAST spaceX);
  xmlNewProp(nodeText,BAD_CAST"y",BAD_CAST spaceY);
  xmlNewProp(nodeText,BAD_CAST"text-anchor",BAD_CAST "middle");
  xmlNewProp(nodeText,BAD_CAST"font-size",BAD_CAST sizeA);
  xmlNewProp(nodeText,BAD_CAST"font-weight",BAD_CAST "bold");
  xmlNewProp(nodeText,BAD_CAST"fill",BAD_CAST "#260A39");

  yPosition = (barLength / 12);
  xPosition = barLength / 6;
  size = barLength * 6 / 150;
  sprintf(sizeA, "%d", size);
  sprintf(spaceX, "%d", xPosition);
  sprintf(spaceY, "%d", yPosition);

  nodeText = xmlNewChild(nodeSVG,NULL, BAD_CAST "text",BAD_CAST axis.yName);
  xmlNewProp(nodeText,BAD_CAST"x",BAD_CAST spaceX);
  xmlNewProp(nodeText,BAD_CAST"y",BAD_CAST spaceY);
  xmlNewProp(nodeText,BAD_CAST"text-anchor",BAD_CAST "middle");
  xmlNewProp(nodeText,BAD_CAST"font-size",BAD_CAST sizeA);
  xmlNewProp(nodeText,BAD_CAST"font-weight",BAD_CAST "bold");

  yPosition =  ((int)atoi(canvas.length) * 7 / 8);
  xPosition = ((int)atoi(canvas.length) * 15 / 16);
  sprintf(spaceX, "%d", xPosition);
  sprintf(spaceY, "%d", yPosition);

  nodeText = xmlNewChild(nodeSVG,NULL, BAD_CAST "text",BAD_CAST axis.xName);
  xmlNewProp(nodeText,BAD_CAST"x",BAD_CAST spaceX);
  xmlNewProp(nodeText,BAD_CAST"y",BAD_CAST spaceY);
  xmlNewProp(nodeText,BAD_CAST"text-anchor",BAD_CAST "middle");
  xmlNewProp(nodeText,BAD_CAST"font-size",BAD_CAST sizeA);
  xmlNewProp(nodeText,BAD_CAST"font-weight",BAD_CAST "bold");

  nodeG = xmlNewChild(nodeSVG, NULL, BAD_CAST "g",NULL);
  xmlNewProp(nodeG,BAD_CAST"stroke-width",BAD_CAST "1.5");

//---------------------------------  BarsDraw  ------------------------------------------------

  int i;
  int j;
  int count = 0;
  int temp = 0;

  int widthAllBar = barLength / numberOfMonth / (numberOfCity+1); 

  int salesArray[numberOfCity*numberOfSales];
  int index=0;
  for(i=0; i<numberOfCity; i++){
    for(j=0; j<numberOfSales;j++){
      salesArray[index]=(int)atoi(cityArray[i].salesArray[j]);
      index++;
    }
  }
  int sizeofArray=0;
  sizeofArray=sizeof(salesArray)/sizeof(salesArray[0]);
  qsort(salesArray,sizeofArray,sizeof(int),compare);

  for (j = 0; j < numberOfCity; j++)
  {

        count= count + barLength/6; //increasing on the x axis
        temp++;

        for (i = 0; i <numberOfSales; i++)
        {
            int valueOfsale = (int)atoi(cityArray[j].salesArray[i]); //sale values
            int spaceFromRight = barLength / 6;
            int lenghtOfRight = barLength + spaceFromRight; //total y axis length
            int valueOfsales = (barLength * valueOfsale) / salesArray[index-1]; //the y axis length of a bar
            int y = lenghtOfRight - valueOfsales; //excess part of a bar
            xPosition = count;

            char bar[12], spaceY[12], spaceW[12],value[12],font[12];
            sprintf(bar, "%d", valueOfsales);
            sprintf(spaceY, "%d", y);
            sprintf(spaceX, "%d", xPosition);
            sprintf(spaceW, "%d", widthAllBar);

            count += widthAllBar *  (numberOfCity+1);
            
            int stroke = atoi(canvas.width) / 300;
            sprintf(strokeSize, "%d", stroke);
            
            nodeRect = xmlNewChild(nodeG, NULL, BAD_CAST "rect", NULL);
            xmlNewProp(nodeRect, BAD_CAST"x", BAD_CAST spaceX);
            xmlNewProp(nodeRect, BAD_CAST"y", BAD_CAST spaceY);
            xmlNewProp(nodeRect, BAD_CAST"width", BAD_CAST spaceW);
            xmlNewProp(nodeRect, BAD_CAST"height", BAD_CAST bar);
            xmlNewProp(nodeRect, BAD_CAST"fill", BAD_CAST cityArray[j].fillcolor);
            xmlNewProp(nodeRect, BAD_CAST"stroke", BAD_CAST "black");
            xmlNewProp(nodeRect,BAD_CAST"stroke-width",BAD_CAST strokeSize);

            if(strcmp(cityArray[j].showvalue,"yes")){
              sprintf(spaceY, "%d", y);
              sprintf(value, "%d", valueOfsale);

              int fontsize= barLength / 50;
              sprintf(font, "%d", fontsize);
              xPosition = xPosition + barLength / 60;
              sprintf(spaceX, "%d", xPosition);

              nodeText = xmlNewChild(nodeG,NULL, BAD_CAST "text", NULL);
              nodeTspan = xmlNewChild(nodeText, NULL, BAD_CAST "tspan", BAD_CAST value);
              xmlNewProp(nodeTspan, BAD_CAST"x", BAD_CAST spaceX);
              xmlNewProp(nodeTspan, BAD_CAST"y", BAD_CAST spaceY);
              xmlNewProp(nodeTspan, BAD_CAST"writing-mode", BAD_CAST "tb");
              xmlNewProp(nodeText, BAD_CAST"font-size", BAD_CAST font);

            }
          }

          count = widthAllBar * temp;
        }

    //----------------------square-------------------------------
        xPosition = barLength * 160 / 150 ;
        yPosition = (barLength * 5) / 150;
        size = barLength * 6.25 / 150;

        sprintf(spaceX, "%d", xPosition);
        sprintf(spaceY, "%d", yPosition);
        sprintf(sizeA, "%d", size);

        for (i = 0; i < numberOfCity; i++)
        {

          nodeRect = xmlNewChild(nodeG, NULL, BAD_CAST "rect", NULL);
          xmlNewProp(nodeRect, BAD_CAST"x", BAD_CAST spaceX);
          xmlNewProp(nodeRect, BAD_CAST"y", BAD_CAST spaceY);
          xmlNewProp(nodeRect, BAD_CAST"width", BAD_CAST sizeA);
          xmlNewProp(nodeRect, BAD_CAST"height", BAD_CAST sizeA);
          xmlNewProp(nodeRect, BAD_CAST"fill", BAD_CAST cityArray[i].fillcolor);
          xmlNewProp(nodeRect, BAD_CAST"stroke", BAD_CAST "black");

          yPosition = yPosition + barLength * 7 / 150;
          sprintf(spaceY, "%d", yPosition);

        }

//--------------------------------- MonthsWrite  ------------------------------------------------

        size = barLength * 5 / 150;
        sprintf(sizeA, "%d", size);
        nodeG = xmlNewChild(nodeSVG, NULL, BAD_CAST "g", NULL);
        xmlNewProp(nodeG, BAD_CAST"font-size", BAD_CAST sizeA);

        nodeText = xmlNewChild(nodeG, NULL, BAD_CAST "text", NULL);

        count = 0;
        yPosition = barLength*177/150;
        sprintf(spaceY, "%d", yPosition);


        for(i = 0; i <numberOfMonth; i++){

          xPosition = barLength * ((barLength / 6 ) + count) / barLength;
          sprintf(spaceX, "%d", xPosition);

          nodeTspan = xmlNewChild(nodeText, NULL, BAD_CAST "tspan", BAD_CAST months.monthArray[i]);
          xmlNewProp(nodeTspan, BAD_CAST"x", BAD_CAST spaceX);
          xmlNewProp(nodeTspan, BAD_CAST"y", BAD_CAST spaceY);
          xmlNewProp(nodeTspan, BAD_CAST"writing-mode", BAD_CAST "tb");
          xmlNewProp(nodeTspan, BAD_CAST"font-weight", BAD_CAST "bold");

          count += widthAllBar *  (numberOfCity+1);

        }

//---------------------------------- SalesWrite   -------------------------------------------------


        nodeText = xmlNewChild(nodeG,NULL, BAD_CAST "text", NULL);
        int countOfy = barLength/6;
    int gecici = salesArray[index-1] + (salesArray[index-1] / numberOfSales); //13125
    int _xPosition = countOfy;
    char geciciArr[10];
    size = barLength*6/150;
    sprintf(sizeA, "%d", size);

    for(i=0; i<numberOfSales; i++){

      xPosition = barLength * 15 / 150;
      sprintf(spaceX, "%d", xPosition);
      yPosition = countOfy;
      sprintf(spaceY, "%d", yPosition);
      gecici = gecici - (salesArray[index-1] / numberOfSales);
      sprintf(geciciArr, "%d", gecici);

      nodeTspan = xmlNewChild(nodeText, NULL, BAD_CAST "tspan", BAD_CAST geciciArr);
      xmlNewProp(nodeTspan, BAD_CAST"x", BAD_CAST spaceX);
      xmlNewProp(nodeTspan, BAD_CAST"y", BAD_CAST spaceY);
      xmlNewProp(nodeTspan, BAD_CAST"text-anchor", BAD_CAST "middle");
      xmlNewProp(nodeTspan, BAD_CAST"font-size", BAD_CAST sizeA);
      xmlNewProp(nodeTspan, BAD_CAST"font-weight", BAD_CAST "bold");

      countOfy += barLength / numberOfSales;

      xPosition = (barLength / 6);
      sprintf(spaceX, "%d", xPosition);
      yPosition = (barLength * 175 / 150);
      sprintf(spaceY, "%d", yPosition);

      sprintf(_spaceX, "%d", _xPosition);

      nodeLine = xmlNewChild(nodeSVG,NULL,BAD_CAST"line", NULL);
      xmlNewProp(nodeLine,BAD_CAST"x1",BAD_CAST spaceX);
      xmlNewProp(nodeLine,BAD_CAST"y1",BAD_CAST _spaceX);
      xmlNewProp(nodeLine,BAD_CAST"x2",BAD_CAST spaceY);
      xmlNewProp(nodeLine,BAD_CAST"y2",BAD_CAST _spaceX);
      xmlNewProp(nodeLine,BAD_CAST"style",BAD_CAST"stroke:rgb(0,0,0);stroke-dasharray:10,2 ;stroke-width:0.5");
      xmlNewProp(nodeLine, BAD_CAST"font-size", BAD_CAST sizeA);
      xmlNewProp(nodeLine, BAD_CAST"font-weight", BAD_CAST "bold");

      _xPosition += (barLength / numberOfSales);

    }
    
//-------------------------------------- Y_Set_NameWrite------------------------------------------------
    
    xPosition = barLength * 170 / 150;
    sprintf(spaceX, "%d", xPosition);
    yPosition = barLength * 10 / 150;
    sprintf(spaceY, "%d", yPosition);

    for(i = 0; i < numberOfCity; i++){

      nodeText = xmlNewChild(nodeG,NULL, BAD_CAST "text", BAD_CAST cityArray[i].name);
      xmlNewProp(nodeText, BAD_CAST"x", BAD_CAST spaceX);
      xmlNewProp(nodeText, BAD_CAST"y", BAD_CAST spaceY);
      xmlNewProp(nodeText, BAD_CAST"font-weight", BAD_CAST "bold");

      yPosition = (barLength * 7 / 150) + yPosition;
      sprintf(spaceY, "%d", yPosition);
    }   

//----------------------------------------- LineDraw ----------------------------------
    
    xPosition= barLength/6;
    _xPosition = barLength + (barLength/6);
    char arrX2[10],arrX1[10];
    sprintf(spaceX,"%d",xPosition);
    sprintf(_spaceX,"%d",_xPosition);

    for (i = 1; i < 3; i++)
    {
      int stroke = atoi(canvas.width) / 200;
      sprintf(strokeSize, "%d", stroke);
      nodeLine = xmlNewChild(nodeSVG,NULL, BAD_CAST "line",NULL);
      xmlNewProp(nodeLine,BAD_CAST"x1",BAD_CAST ((i%2) ? spaceX :spaceX ));
      xmlNewProp(nodeLine,BAD_CAST"y1",BAD_CAST ((i%2) ? _spaceX :spaceX ));
      xmlNewProp(nodeLine,BAD_CAST"x2",BAD_CAST ((i%2) ? _spaceX :spaceX ));
      xmlNewProp(nodeLine,BAD_CAST"y2",BAD_CAST ((i%2) ? _spaceX :_spaceX ));
      xmlNewProp(nodeLine,BAD_CAST"stroke",BAD_CAST "black");
      xmlNewProp(nodeLine,BAD_CAST"stroke-width",BAD_CAST strokeSize);

    }

    xmlSaveFormatFileEnc(outputName, doc, "UTF-8", 1);
  }
  int compare(const void * a,const void * b){
    return(*(int*)a - *(int*)b );
  }


  void printHelp()
  { printf("\n\n\t------------------------HELP---------------------------\n\n");
  printf("Correct input type: \n\n");
  printf("-i <XML Filename> -o <Output Filename> -v <XSD Validation Filename> -t <Type> \n");
  printf("-t <Type> : 'line' | 'pie' | 'bar' \n\n");
}

void freeMemory()
{
  int i, j;
  free(chartgendata.charttitle);
  free(canvas.length);
  free(canvas.width);
  free(canvas.backcolor);
  free(axis.xName);
  free(axis.yName);
  free(axis.yForecolor);
  free(months.monthArray);
  for(i=0; i < numberOfCity; i++)
  {
    free(cityArray[i].unit);
    free(cityArray[i].name);
    free(cityArray[i].fillcolor);

    for(j=0; j < numberOfSales; j++)
    {
      free(cityArray[i-1].salesArray[j]);
    }
  }
  
}
