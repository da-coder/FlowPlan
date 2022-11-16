// Sean Slater
// 2020/01/20
// DegreeFlow / FlowPlan Header

// define variables
#define MAX_INPUT_LINE 2048
#define MAX_INPUT_CHAR 64
#define MAX_NAME_LEN 9

// define structs
struct user_specifications {
	// used to send variables to getInput
	int number_of_courses;								// number of courses detected
	int max_hours_per_sem;								// user entered number of hours per semester
	int summer_user_choice;								// 0=no, >0 means yes where number = hours in summer
	int starting_year;									// user specified starting year
	int starting_season;								// user specified starting season
	char* degreeplan_filename;							// filename of degree plan to load
	char* credits_filename;								// filename of credits to load
};
struct course {
	// individual course struct
	char name[MAX_NAME_LEN];	// name of course
	int hours;					// number of hours per week
	int status;					// 0=no credit, 1=current, 2=credit
	int ava_Fall;				// flag if this course is available during fall
	int ava_Spring;				// flag if this course is available during spring
	int ava_Summer;				// flag if this course is available during summer

	// variables for linked list
	int prereqs_num;			// number of prereqs
	int coreqs_num;				// number of coreqs
	struct course** prereqs;	// points to any courses that are prereqs
	struct course** coreqs;		// points to any courses that are coreqs

	// empty char array for saving reqs before they can be pointers
	char temp[MAX_INPUT_CHAR];
};
struct semester {
	// individual semester struct
	int year;					// year, 4 digits
	int season;					// 0=fall, 1=spring, 2=summer
	int number_of_courses;		// tracker of number of current courses
	int hours_left;				// number of hours left in this semester
	struct course** courses;	// courses in this semester

	// pointers for linked list
	struct semester* next;		// points to the next semester, NULL if last
	struct semester* prev;		// points to the prev semester, NULL if first
};
struct chain_link {
	// a single chain link of chain
	int count;					// count of how many chain links precede it
	struct course* c;			// points to course for this link in chain

	// pointers for linked list
	struct chain_link* next;	// points to next in chain, NULL if last
};
struct chain {
	int number_of_chains;		// number of chain starts
	struct chain_link* c_l;		// array of chain link starts
};

// define methods
char* rmNewline(char* string);
int getInput(struct user_specifications* u_s, int argc, char* argv[]);
struct course* createCourse(char* name, int hours);
struct semester* createSemester(int y, int s, int h, struct semester* n, struct semester* p);
struct chain_link* createChain_Link(int count, struct course* c, struct chain_link* n);
int readDegreePlan(char* filename, struct course** courses);
int parseReqs(struct course* courses, int n);
int readCoursesCredits(struct course* courses, char* filename, int n);
int checkStatus(struct course* c);
int isPrereq(struct course* p, struct course* c);
int isCoreq(struct course* p, struct course* c);
int addCourseToSemester(struct semester* s, struct course* c);
int buildChain(struct chain* chains, struct chain_link* next, struct course* c, int count);
struct semester* calculatePlan(struct course* courses, struct user_specifications* u_s);
int savePlan();
int freeMem();
