/* minunit_test.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include "minunit.h"
#include "twkb.h"

int tests_run = 0;


/*Test unsigned varint readings*/
static uint64_t test_uvarint(uint64_t in) {
    TWKB_BUF *tb;
    uint64_t res;
    tb = malloc(sizeof(TWKB_BUF));

    tb->read_pos = (uint8_t*) &in;
    tb->end_pos =  (uint8_t*) (tb->read_pos+8);
    res = buffer_read_uvarint(tb) ;
    free (tb);
    return res;
}

static char * test_uvarint1() {
    mu_assert("error, test_uvarint1", test_uvarint(0x01) == 1);
    return 0;
}
static char * test_uvarint2() {
    mu_assert("error, test_uvarint2", test_uvarint(0x01c8) == 200);
    return 0;
}

/*Test signed varint readings*/
static int64_t test_svarint(uint64_t in) {
    TWKB_BUF *tb;
    int64_t res;
    tb = malloc(sizeof(TWKB_BUF));

    tb->read_pos = (uint8_t*) &in;
    tb->end_pos =  (uint8_t*) (tb->read_pos+8);
    res = buffer_read_svarint(tb);
    free (tb);
    return res ;
}

static char * test_svarint1() {
    mu_assert("error, test_svarint1", test_svarint(0x26) == 19);
    return 0;
}
static char * test_svarint2() {
    mu_assert("error, test_svarint2", test_svarint(0x0390) == 200);
    return 0;
}

static char * test_svarint3() {
    mu_assert("error, test_svarint3", test_svarint(0x01) == -1);
    return 0;
}

static char * test_svarint4() {
    mu_assert("error, test_svarint4", test_svarint(0x01c7) == -100);
    return 0;
}
static char * test_svarint5() {
    mu_assert("error, test_svarint4", test_svarint(0x04) == 2);
    return 0;
}

static uint8_t* hex2intbuf(const char * hexstr,size_t *size)
{
    uint8_t *res;
    size_t len = strlen(hexstr);
    *size=len/2;
    char c[4];
	
    int i;
    res = malloc(len/2);

    *res=0;
    for (i=0; i<len/2; i++)
    {
	snprintf(c,3,"%s",hexstr +2*i);
       res[i] = (uint8_t) strtoul(c,NULL,16);
    }
    return res;
}




static GEOM * test_geoms(char* hexstr,buffer_collection *res_buf) {
    GEOM *g;
    uint8_t *in;
    size_t size;
    in = hex2intbuf(hexstr, &size);

    g=decode_twkb_start((uint8_t*) in,size,res_buf);
    free(in);
    return g;
}


/********************************************************************************
Start testing geoJSON
*********************************************************************************/
static char * test_geoJSON_point1(buffer_collection *res_buf) {

    GEOM *g = (GEOM*) test_geoms("01000202",res_buf);
    mu_assert("error, test_point1_1",g->type == 1);
    mu_assert("error, test_point1_2",((POINT*) g)->point->npoints == 1);

    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"Point\",\"coordinates\":[1,1]}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_point1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_geoJSON_line1(buffer_collection *res_buf) {
    GEOM *g = test_geoms("020003040200020104",res_buf);
    mu_assert("error, test_geoJSON_line1_1",g->type == 2);
    mu_assert("error, test_geoJSON_line1_2",((LINE*) g)->points->npoints == 3);

    char *txt = encode_geojson_start(g,res_buf);
	
    char *cmp_txt="{\"type\":\"LineString\",\"coordinates\":[[2,1],[2,2],[1,4]]}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_line1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}
static char * test_geoJSON_polygon1(buffer_collection *res_buf) {
    GEOM *g =test_geoms("0300010502020002020000010100",res_buf);
    mu_assert("error, test_geoJSON_polygon1_1",g->type == 3);
    mu_assert("error, test_geoJSON_polygon1_2",( (POLY*) g)->nrings == 1);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"Polygon\",\"coordinates\":[[[1,1],[1,2],[2,2],[2,1],[1,1]]]}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_polygon1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}


static char * test_geoJSON_polygon2(buffer_collection *res_buf) {
    GEOM *g = test_geoms("0300020502021226140000132511040808000202000101",res_buf);
    mu_assert("error, test_geoJSON_polygon2_1",g->type == 3);
    mu_assert("error, test_geoJSON_polygon2_2",((POLY*) g)->nrings == 2);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"Polygon\",\"coordinates\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]]}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_polygon2_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_geoJSON_collection1(buffer_collection *res_buf) {
    GEOM *g = test_geoms("04000202020202",res_buf);
    mu_assert("error, test_geoJSON_collection1_1",g->type == 7);
    mu_assert("error, test_geoJSON_collection1_2",((COLLECTION*) g)->ngeoms == 2);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"GeometryCollection\",\"geometries\":[{\"type\":\"Point\",\"coordinates\":[1,1]},{\"type\":\"Point\",\"coordinates\":[2,2]}}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_collection1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_geoJSON_collection2(buffer_collection *res_buf) {
    GEOM *g = test_geoms("050002030402000201040210080b0b",res_buf);
    mu_assert("error, test_geoJSON_collection2_1",g->type == 7);
    mu_assert("error, test_geoJSON_collection2_2",((COLLECTION*) g)->ngeoms == 2);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"GeometryCollection\",\"geometries\":[{\"type\":\"LineString\",\"coordinates\":[[2,1],[2,2],[1,4]]},{\"type\":\"LineString\",\"coordinates\":[[9,8],[3,2]]}}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_collection2_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_geoJSON_collection3(buffer_collection *res_buf) {
    GEOM *g = test_geoms("06000202050202122614000013251104080800020200010101040303000202000101",res_buf);
    mu_assert("error, test_geoJSON_collection3_1",g->type == 7);
    mu_assert("error, test_geoJSON_collection3_2",((COLLECTION*) g)->ngeoms == 2);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"GeometryCollection\",\"geometries\":[{\"type\":\"Polygon\",\"coordinates\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]]},{\"type\":\"Polygon\",\"coordinates\":[[[3,3],[3,4],[4,4],[3,3]]]}}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_collection3_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_geoJSON_collection4(buffer_collection *res_buf) {
    GEOM *g = test_geoms("070003010002020200030402000201040300020502021226140000132511040808000202000101",res_buf);
    mu_assert("error, test_geoJSON_collection4_4",g->type == 7);
    mu_assert("error, test_geoJSON_collection4_4",((COLLECTION*) g)->ngeoms == 3);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"GeometryCollection\",\"geometries\":[{\"type\":\"Point\",\"coordinates\":[1,1]},{\"type\":\"LineString\",\"coordinates\":[[2,1],[2,2],[1,4]]},{\"type\":\"Polygon\",\"coordinates\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]]}}";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_collection4_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}


static char * test_geoJSON_collection5(buffer_collection *res_buf) {
    GEOM *g = test_geoms("070403020406010002020200030402000201040300020502021226140000132511040808000202000101",res_buf);
    mu_assert("error, test_geoJSON_collection5_4",g->type == 7);
    mu_assert("error, test_geoJSON_collection5_4",((COLLECTION*) g)->ngeoms == 3);
    char *txt = encode_geojson_start(g,res_buf);
    char *cmp_txt="{\"type\":\"FeatureCollection\",\"features\":[{\"type\":\"Feature\",\"properties\":{\"id\":1},\"geometry\":{\"type\":\"Point\",\"coordinates\":[1,1]}},{\"type\":\"Feature\",\"properties\":{\"id\":2},\"geometry\":{\"type\":\"LineString\",\"coordinates\":[[2,1],[2,2],[1,4]]}},{\"type\":\"Feature\",\"properties\":{\"id\":3},\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]]}}}";
    printf("%s\n",txt);
	
    mu_assert("error, test_geoJSON_collection5_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}


static char * test_geoJSON_many_geometries1(buffer_collection *res_buf) {
    size_t size;
   uint8_t *buf = hex2intbuf("0100020201000202", &size);
	
    char *txt = twkb2geoJSON(buf,size);
    char *cmp_txt="[{\"type\":\"Point\",\"coordinates\":[1,1]},{\"type\":\"Point\",\"coordinates\":[1,1]}]";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_many_geometries1_1",(strcmp(txt, cmp_txt))==0);
    free(txt);
free(buf);
    return 0;
}

static char * test_geoJSON_n_decimals(buffer_collection *res_buf) {
    size_t size;
   uint8_t *buf = hex2intbuf("a504012e04eea7800102000202040102", &size);   
    char *txt = twkb2geoJSON(buf,size);
    char *cmp_txt="[{\"type\":\"Feature\",\"properties\":{\"id\":23},\"geometry\":{\"type\":\"LineString\",\"coordinates\":[[10.51127,0.00001],[10.51127,0.00002],[10.51128,0.00004],[10.51127,0.00005]]}}]";
    printf("%s\n",txt);
    mu_assert("error, test_geoJSON_n_decimals_1",(strcmp(txt, cmp_txt))==0);
    free(txt);
free(buf);
    return 0;
}

static char * test_filereading2geoJSON(buffer_collection *res_buf) { 
	char *file_name = "testdata/kommuner.twkb";	
	char *txt = twkb2geoJSON_fromFile(file_name);
	printf("%s\n",txt);
	free(txt);
	return 0;
}



/********************************************************************************
Start testing esri JSON
*********************************************************************************/

static char * test_esrijson_point1(buffer_collection *res_buf) {

    GEOM *g = (GEOM*) test_geoms("01000202",res_buf);
    mu_assert("error,test_esrijson_point1_1",g->type == 1);
    mu_assert("error, test_esrijson_point1_2",((POINT*) g)->point->npoints == 1);

    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"x\":1,\"y\":1,\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_point1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_esrijson_line1(buffer_collection *res_buf) {
    GEOM *g = test_geoms("020003040200020104",res_buf);
    mu_assert("error, test_esrijson_line1_1",g->type == 2);
    mu_assert("error, test_esrijson_line1_2",((LINE*) g)->points->npoints == 3);

    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"paths\":[[[2,1],[2,2],[1,4]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_line1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_esrijson_polygon1(buffer_collection *res_buf) {
    GEOM *g =test_geoms("0300010502020002020000010100",res_buf);
    mu_assert("error, test_esrijson_polygon1_1",g->type == 3);
    mu_assert("error, test_esrijson_polygon1_2",( (POLY*) g)->nrings == 1);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"rings\":[[[1,1],[1,2],[2,2],[2,1],[1,1]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_polygon1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}


static char * test_esrijson_polygon2(buffer_collection *res_buf) {
    GEOM *g = test_geoms("0300020502021226140000132511040808000202000101",res_buf);
    mu_assert("error, test_esrijson_polygon2_1",g->type == 3);
    mu_assert("error, test_esrijson_polygon2_2",((POLY*) g)->nrings == 2);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"rings\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_polygon2_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_esrijson_collection1(buffer_collection *res_buf) {
    GEOM *g = test_geoms("04000202020202",res_buf);
    mu_assert("error, test_esrijson_collection1_1",g->type == 7);
    mu_assert("error, test_esrijson_collection1_2",((COLLECTION*) g)->ngeoms == 2);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"x\":1,\"y\":1,\"spatialReference\":0}},{\"geometry\":{\"x\":2,\"y\":2,\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_collection1_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_esrijson_collection2(buffer_collection *res_buf) {
    GEOM *g = test_geoms("050002030402000201040210080b0b",res_buf);
    mu_assert("error, test_esrijson_collection2_1",g->type == 7);
    mu_assert("error, test_esrijson_collection2_2",((COLLECTION*) g)->ngeoms == 2);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"paths\":[[[2,1],[2,2],[1,4]]],\"spatialReference\":0}},{\"geometry\":{\"paths\":[[[9,8],[3,2]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_collection2_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_esrijson_collection3(buffer_collection *res_buf) {
    GEOM *g = test_geoms("06000202050202122614000013251104080800020200010101040303000202000101",res_buf);
    mu_assert("error, test_esrijson_collection3_1",g->type == 7);
    mu_assert("error, test_esrijson_collection3_2",((COLLECTION*) g)->ngeoms == 2);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"rings\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]],\"spatialReference\":0}},{\"geometry\":{\"rings\":[[[3,3],[3,4],[4,4],[3,3]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_collection3_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}

static char * test_esrijson_collection4(buffer_collection *res_buf) {
    GEOM *g = test_geoms("070003010002020200030402000201040300020502021226140000132511040808000202000101",res_buf);
    mu_assert("error, test_esrijson_collection4_1",g->type == 7);
    mu_assert("error, test_esrijson_collection4_2",((COLLECTION*) g)->ngeoms == 3);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"geometry\":{\"x\":1,\"y\":1,\"spatialReference\":0}},{\"geometry\":{\"paths\":[[[2,1],[2,2],[1,4]]],\"spatialReference\":0}},{\"geometry\":{\"rings\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_collection4_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}


static char * test_esrijson_collection5(buffer_collection *res_buf) {
    GEOM *g = test_geoms("070403020406010002020200030402000201040300020502021226140000132511040808000202000101",res_buf);
    mu_assert("error, test_esrijson_collection5_1",g->type == 7);
    mu_assert("error, test_esrijson_collection5_2",((COLLECTION*) g)->ngeoms == 3);
    char *txt = encode_esrijson_start(g,0, res_buf);
    char *cmp_txt="{\"GlobalId\":1,\"geometry\":{\"x\":1,\"y\":1,\"spatialReference\":0}},{\"GlobalId\":2,\"geometry\":{\"paths\":[[[2,1],[2,2],[1,4]]],\"spatialReference\":0}},{\"GlobalId\":3,\"geometry\":{\"rings\":[[[1,1],[10,20],[20,20],[20,10],[1,1]],[[5,5],[5,6],[6,6],[5,5]]],\"spatialReference\":0}}";
    printf("%s\n",txt);
	
    mu_assert("error, test_esrijson_collection5_3",(strcmp(txt, cmp_txt))==0);
    free(txt);
    return 0;
}


static char * test_esrijson_many_geometries1(buffer_collection *res_buf) {
    size_t size;
   uint8_t *buf = hex2intbuf("0100020201000202", &size);
    char *txt = twkb2esriJSON(buf,size,32633);
    char *cmp_txt="[{\"geometry\":{\"x\":1,\"y\":1,\"spatialReference\":32633}},{\"geometry\":{\"x\":1,\"y\":1,\"spatialReference\":32633}}]";
    //~ printf("%s\n",txt);
    mu_assert("error, test_esrijson_many_geometries1_1",(strcmp(txt, cmp_txt))==0);
    free(txt);
free(buf);
    return 0;
}

static char * test_esrijson_n_decimals(buffer_collection *res_buf) {
    size_t size;
   uint8_t *buf = hex2intbuf("a504012e04eea7800102000202040102", &size);
    char *txt = twkb2esriJSON(buf,size,4326);
    char *cmp_txt="[{\"GlobalId\":23,\"geometry\":{\"paths\":[[[10.51127,0.00001],[10.51127,0.00002],[10.51128,0.00004],[10.51127,0.00005]]],\"spatialReference\":4326}}]";
    printf("%s\n",txt);
    mu_assert("error, test_esrijson_n_decimals_1",(strcmp(txt, cmp_txt))==0);
    free(txt);
free(buf);
    return 0;
}


static char * test_esrijson_filereading(buffer_collection *res_buf) { 
	char *file_name = "testdata/kommuner.twkb";	
	char *txt = twkb2esriJSON_fromFile(file_name,32633);
	printf("%s\n",txt);
	free(txt);
	return 0;
}


static char * all_tests() {
int i;
    printf("---------------------------------------------------\n");
    buffer_collection res_buf;
	for (i=0;i<1;i++)
	{
    init_res_buf(&res_buf);
    mu_run_test(test_uvarint1,&res_buf);
    mu_run_test(test_uvarint2,&res_buf);
    mu_run_test(test_svarint1,&res_buf);
    mu_run_test(test_svarint2,&res_buf);
    mu_run_test(test_svarint3,&res_buf);
    mu_run_test(test_svarint4,&res_buf);
    mu_run_test(test_svarint5,&res_buf);
	
	
    mu_run_test(test_geoJSON_point1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_line1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_polygon1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_polygon2,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_collection1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_collection2,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_collection3,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_collection4,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_collection5,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_many_geometries1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_geoJSON_n_decimals,&res_buf);
    printf("---------------------------------------------------\n");
	
    mu_run_test(test_esrijson_point1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_line1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_polygon1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_polygon2,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_collection1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_collection2,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_collection3,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_collection4,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_collection5,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_many_geometries1,&res_buf);
    printf("---------------------------------------------------\n");
    mu_run_test(test_esrijson_n_decimals,&res_buf);
    printf("---------------------------------------------------\n");
    
    
  mu_run_test(test_filereading2geoJSON,&res_buf);
 //   mu_run_test(test_filereading,&res_buf);
    destroy_buffer(&res_buf);
    }
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("-----------------------------------------\nALL TESTS PASSED\n--------------------------------------------\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}