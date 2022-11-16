// Sean Slater
// 2020/01/20
// DegreeFlow / FlowPlan Source Code

// last left off:
// need to check, recursively, prereqs to critical path (farthest first)
// difficulty option
// courses can't be taken in summer/spring/fall
// instability in release (but not in debug) from variable initializion

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FlowPlan.h"

int main(int argc, char* argv[]) {
	// define variables for passing
	struct user_specifications* u_s;					// struct for easier passing of information
	struct course* courses;								// pointer to array of all courses
	struct semester* semesters;							// pointer to array of semesters

	courses = malloc(sizeof(struct course));			// malloc for realloc later
	u_s = malloc(sizeof(struct user_specifications));	// malloc to guarantee memory

	if (argc == 2 && strcmp(argv[1], "help") == 0) {
		printf("usage: FlowPlan <degree plan filename> <transcript filename> <max hours> <summer hours>\n");
		printf("program can ingest 0, 1, 2, 3, or all 4 arguments (must remain in order)\n");
		printf(" it will ask for anything it is missing\n");
		return 1;
	}
	if (argc == 2 && strcmp(argv[1], "about") == 0) {
		printf("Sean Slater (c) 2021\n");
		printf("FlowPlan\n");
		printf(" recommends an optimal path to graduation\n");
		return 1;
	}

	getInput(u_s, argc, argv);							// handle user input
	if (u_s->max_hours_per_sem > 20 || u_s->max_hours_per_sem < 1) {
		printf("error: max hours out of range\nexiting...\n");
		return 1;
	}
	if (u_s->summer_user_choice > 20 || u_s->summer_user_choice < 0) {
		printf("error: summer hours out of range\nexiting...\n");
		return 1;
	}
	u_s->number_of_courses = readDegreePlan(u_s->degreeplan_filename, &courses);
														// reads the degree plan file
	if (!(u_s->number_of_courses)) {
		printf("exiting...\n");
		return 1;
	}
	parseReqs(courses, u_s->number_of_courses);			// parses the reqs
	readCoursesCredits(courses, u_s->credits_filename, u_s->number_of_courses);
														// read credits file and set status
	semesters = calculatePlan(courses, u_s);			// create program plan!


	// DEBUG : REMOVE LATER : prints all semesters
	while (semesters->next != NULL) {
		switch (semesters->season) {
			case 0:
				printf("%d Fall:\n", semesters->year);
				break;
			case 1:
				printf("%d Spring:\n", semesters->year);
				break;
			case 2:
				printf("%d Summer:\n", semesters->year);
				break;
		}

		int i;
		for (i=0; i<semesters->number_of_courses; i++) {
			printf(" +-- %s\n", semesters->courses[i]->name);
		}
		semesters = semesters->next;
	}

	// finish
	return 0;
}

char* rmNewline(char* string) {
	// removes newline, if it exists, from end of string
	if (string == NULL) {								// checks for valid pointer
		printf("error: rmNewline: string does not exist!\n");
		return NULL;									// not a valid pointer
	}

	int index = strlen(string) - 1;						// get index of possible '\n'
	if (string[index] == '\n')							// check index for '\n'
		string[index] = '\0';							// set new terminator

	return string;										// success
}

int getInput(struct user_specifications* u_s, int argc, char* argv[]) {
	// gets user input such as hours / semester and is summer ok
	char buffer[MAX_INPUT_CHAR] = "";

	switch (argc) {
		case 1:
			// no command line arguments
			goto askall;
			break;
		case 2:
			// one command line argument, degree plan
			u_s->degreeplan_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->degreeplan_filename, argv[1]);
			goto askmost;
			break;
		case 3:
			// two command line arguments, degree plan and credit file
			u_s->degreeplan_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->degreeplan_filename, argv[1]);
			u_s->credits_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->credits_filename, argv[2]);
			goto asksome;
			break;
		case 4:
			// three command line arguments, degree plan, credit file, and max hours
			u_s->degreeplan_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->degreeplan_filename, argv[1]);
			u_s->credits_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->credits_filename, argv[2]);
			u_s->max_hours_per_sem = atoi(argv[3]);
			goto askless;
			break;
		case 5:
			// four command line arguments, degree plan, credit file, max hours, and summer hours
			u_s->degreeplan_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->degreeplan_filename, argv[1]);
			u_s->credits_filename = malloc(strlen(buffer)+1);
			strcpy(u_s->credits_filename, argv[2]);
			u_s->max_hours_per_sem = atoi(argv[3]);
			u_s->summer_user_choice = atoi(argv[4]);
			goto asknone;
			break;
	}

	askall:
	printf("filename of degree plan: ");
	fgets(buffer, MAX_INPUT_CHAR, stdin);
	strcpy(buffer, rmNewline(buffer));
	u_s->degreeplan_filename = malloc(strlen(buffer)+1);// no need for sizeof char, it's a byte
	strcpy(u_s->degreeplan_filename, buffer);			// copy buffer into struct->char

	askmost:
	printf("filename of credits: ");
	fgets(buffer, MAX_INPUT_CHAR, stdin);
	strcpy(buffer, rmNewline(buffer));
	u_s->credits_filename = malloc(strlen(buffer)+1);
	strcpy(u_s->credits_filename, buffer);

	asksome:
	printf("number of hours per normal semester allowed: ");
	fgets(buffer, MAX_INPUT_CHAR, stdin);
	strcpy(buffer, rmNewline(buffer));					// remove newline
	u_s->max_hours_per_sem = atoi(buffer);

	askless:
	printf("number of hours per summer allowed: ");
	fgets(buffer, MAX_INPUT_CHAR, stdin);
	strcpy(buffer, rmNewline(buffer));
	u_s->summer_user_choice = atoi(buffer);				// if it's wrong it will be 0

	asknone:
	printf("starting semsester (year): ");
	fgets(buffer, MAX_INPUT_CHAR, stdin);
	strcpy(buffer, rmNewline(buffer));
	u_s->starting_year = atoi(buffer);
	printf("starting semsester (season): ");
	fgets(buffer, MAX_INPUT_CHAR, stdin);
	strcpy(buffer, rmNewline(buffer));
	if (strcmp(buffer, "fall") <= 0) {					// first unmatched char can handle case
		u_s->starting_season = 0;						// set to fall
	} else if (strcmp(buffer, "spring") <= 0) {			// needs to be in alphabetical order
		u_s->starting_season = 1;
	} else if (strcmp(buffer, "summer") <= 0) {
		u_s->starting_season = 2;
	}													// this will return a 2 for any random text

	return 0;											// success
}

struct course* createCourse(char* name, int hours) {
	// creates a course and sets its variables
	struct course* temp = malloc(sizeof(struct course));
	strcpy(temp->name, name);							// set variables on new course
	temp->hours = hours;
	temp->status = 0;									// set class as uncredited by default
	temp->prereqs_num = 0;								// set number of reqs to 0 by default
	temp->coreqs_num = 0;
	temp->prereqs = malloc(sizeof(struct course*));		// initial malloc for realloc later
	temp->coreqs = malloc(sizeof(struct course*));

	return temp;										// return pointer to new course
}

struct semester* createSemester(int y, int s, int h, struct semester* n, struct semester* p) {
	// creates a semester and returns pointer to it
	struct semester* nu_semester = malloc(sizeof(struct semester));
	nu_semester->year = y;								// save year
	nu_semester->season = s;							// save season
	nu_semester->number_of_courses = 0;					// new semesters have no courses
	nu_semester->hours_left = h;						// set new semester hours
	nu_semester->courses = malloc(sizeof(struct course*));
														// for realloc later
	nu_semester->next = n;								// save next
	nu_semester->prev = p;								// save prev

	return nu_semester;									// return pointer to new semester
}

struct chain_link* createChain_Link(int count, struct course* c, struct chain_link* n) {
	// creates a chain link
	struct chain_link* c_l = malloc(sizeof(struct chain_link));
	c_l->count = count;									// save count
	c_l->c = c;											// set course pointer
	c_l->next = n;										// set next pointer

	return c_l;											// return pointer to new chain link
}

int readDegreePlan(char* filename, struct course** courses) {
	// reads a user-created degree plan file
	int hours = 0;
	int offset = 0;
	char* delim = ",";
	char* index;
	char name[MAX_NAME_LEN] = "";
	char buffer[MAX_INPUT_CHAR] = "";

	// attempt to open file
	FILE* fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {									// check if file is valid
		printf("error: unable to find file \"%s\"\n", filename);
		return -1;										// failed to find file
	}

	// read file into memory
	int n = 0;
	while (fgets(buffer, MAX_INPUT_CHAR, fp) != NULL) {
		// each line is: name,hours,prereq,prereq,...;coreq,coreq,...
		strcpy(buffer, rmNewline(buffer));				// remove newline
		strcpy(name, strtok(buffer, delim));			// save name
		hours = atoi(strtok(NULL, delim));				// save hours
														// FUTURE: impliment checking here
		*courses = realloc(*courses, sizeof(struct course) * (n+1));
														// resize array for new course
		(*courses)[n] = *createCourse(name, hours);		// create a course and add to array
		offset = strlen(name) + 3;						// calculate offset (name length + single digit + 2 commas)
		index = buffer+offset;							// apply offset
		strcpy((*courses)[n].temp, index);				// copy everything after comma to temp
		n++;											// iterate
	}

	// close file
	fclose(fp);

	return n;											// success, return number of courses read
}

int parseReqs(struct course* courses, int n) {
	// using data in course struct -> temp, set preqs and coreqs
	char* delim_comma = ",";
	char* delim_semicolon = ";";
	char* name;
	int i, j, k, index, is_found = 0;

	// handle prereqs
	for (i=0; i<n; i++) {								// iterate over each course
		// split prereqs and coreqs
		char p[MAX_INPUT_LINE] = "";
		char c[MAX_INPUT_LINE] = "";
		name = strtok(courses[i].temp, delim_semicolon);
		if (courses[i].temp[0] == ';') {
			name = NULL;								// this is to get around strtok skipping first delim
		}
		if (name != NULL) {
			strcpy(p, name);							// copy prereqs into string
		}
		name = strtok(NULL, delim_semicolon);
		if (name != NULL) {
			strcpy(c, name);							// copy coreqs into string
		}

		// handle prereqs
		name = strtok(p, delim_comma);					// tokenize and find name (prereqs)
		while (name != NULL) {							// iterate over remaining tokens ()
			is_found = 0;								// reset boolean
			for (j=0; j<n; j++) {						// iterate over each course
				if (strcmp(courses[j].name, name) == 0) {
					courses[i].prereqs_num++;			// iterate number
					courses[i].prereqs = realloc(courses[i].prereqs, \
					 sizeof(struct course*) * courses[i].prereqs_num);
														// allocate space
					courses[i].prereqs[courses[i].prereqs_num - 1] = &(courses[j]);
														// save pointer to this course
					is_found = 1;						// set boolan
				}
			}

			if (!is_found)								// check boolean
				printf("error: unable to find prereq: %s\n", name);

			name = strtok(NULL, delim_comma);			// grab next token
		}

		// handle coreqs
		name = strtok(c, delim_comma);					// tokenize and find name (prereqs)
		while (name != NULL) {							// iterate over remaining tokens ()
			is_found = 0;								// reset boolean
			for (j=0; j<n; j++) {						// iterate over each course
				if (strcmp(courses[j].name, name) == 0) {
					courses[i].coreqs_num++;			// iterate number
					courses[i].coreqs = realloc(courses[i].coreqs, \
					 sizeof(struct course*) * courses[i].coreqs_num);
														// allocate space
					courses[i].coreqs[courses[i].coreqs_num - 1] = &(courses[j]);
														// save pointer to this course
					is_found = 1;						// set boolan
				}
			}

			if (!is_found)								// check boolean
				printf("error: unable to find coreq: %s\n", name);

			name = strtok(NULL, delim_comma);			// grab next token
		}
	}

	return 0;											// success
}

int readCoursesCredits(struct course* courses, char* filename, int n) {
	// reads file and sets status accordingly
	char buffer[MAX_INPUT_LINE];
	char* name;
	char* delim = ",";
	int i, is_found;

	// attempt to open file
	FILE* fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {									// check if file is valid
		printf("error: unable to find file \"%s\"\n", filename);
		return -1;										// failed to find file
	}

	// file will be "course,course,course,course,...,course"
	// read file into memory
	fgets(buffer, MAX_INPUT_LINE, fp);					// read line
	strcpy(buffer, rmNewline(buffer));					// remove newline
	name = strtok(buffer, delim);						// tokenize first course
	while (name != NULL) {								// iterate over tokens
		is_found = 0;									// reset boolean
		for (i=0; i<n; i++) {							// iterate over each course
			if (strcmp(courses[i].name, name) == 0) {	// check if this is the course
				is_found = 1;							// set boolean
				courses[i].status = 2;					// set as taken
				printf(" + credit for %s\n", courses[i].name);
			}
		}

		if (!is_found)									// check boolean
			printf("error: unable to find course for credit: %s\n", name);

		name = strtok(NULL, delim);						// grab next token
	}

	// close file
	fclose(fp);

	return 0;											// success
}

int checkStatus(struct course* c) {
	// returns if course is eligible to be taken
	if (c->status != 0) {
		return 1;										// class is current or already taken
	}

	int i;
	for (i=0; i<c->prereqs_num; i++) {					// iterate over each prereq
		if (c->prereqs[i]->status != 2) {
			return 1;									// prereq is not taken
		}
	}
	for (i=0; i<c->coreqs_num; i++) {					// iterate over each coreq
		if (c->coreqs[i]->status == 0) {
			return 1;									// coreq is not current nor taken
		}
	}

	return 0;											// class is eligible
}

int isPrereq(struct course* p, struct course* c) {
	// returns if p is a prereq of c
	int i;
	for (i=0; i<c->prereqs_num; i++) {					// iterate over prereqs
		if (c->prereqs[i] == p) {
			return 1;									// found that it is a prereq
		}
	}

	return 0;											// found no match
}

int isCoreq(struct course* p, struct course* c) {
	// returns if p is a coreq of c
	int i;
	for (i=0; i<c->coreqs_num; i++) {					// iterate over coreqs
		if (c->coreqs[i] == p) {
			return 1;									// found that it is a coreq
		}
	}

	return 0;											// found no match
}

int addCourseToSemester(struct semester* s, struct course* c) {
	// attempts to add specified course to semester
	// returns -1 for error, 0 for success, 1 for prereqs missing, 2 for not enough hours
	// check if both are valid
	if (s == NULL || c == NULL)
		return -1;										// failure, one pointer is NULL

	// check if enough hours
	if (s->hours_left < c->hours)						// will this class fit within specified max?
		return 2;										// not enough hours

	// check if prereqs met
	if (checkStatus(c))
		return 1;										// prereq or coreq not met

	// add course to semester
	s->courses = realloc(s->courses, sizeof(struct course*) * (s->number_of_courses+1));
														// reallocate to allow space
	s->courses[s->number_of_courses] = c;				// save course in array
	s->number_of_courses++;								// increment counter
	s->hours_left -= c->hours;							// subtract course hours from total hours

	c->status = 1;										// set course as current

	// return
	return 0;											// success
}

int buildChain(struct chain* chains, struct chain_link* next, struct course* c, int count) {
	// navigates down prereqs until it reaches bottom building a chain
	count++;											// iterate count
	
	next = createChain_Link(count, c, next);			// create chain link, update next

	int i;
	for (i=0; i<c->prereqs_num; i++) {					// iterate over each prereq
		buildChain(chains, next, c->prereqs[i], count);	// navigate to bottom first
	}

	if (!c->prereqs_num) {								// is this a leaf? (has no children)
		chains->number_of_chains++;						// iterate counter
		chains->c_l = realloc(chains->c_l, sizeof(struct chain_link) * chains->number_of_chains);
														// adjust allocation to match
		chains->c_l[chains->number_of_chains-1] = *next;// save this chain link to array
	}

	return count;
}

struct semester* calculatePlan(struct course* courses, struct user_specifications* u_s) {
	// takes courses and creates then returns optimal semesters
	// initialize variables
	int m = u_s->max_hours_per_sem;						// pull specs out of struct
	int s = u_s->summer_user_choice;
	int n = u_s->number_of_courses;
	int year = u_s->starting_year;
	int sea = u_s->starting_season;
	struct chain* chains = malloc(sizeof(struct chain*));
														// chains is an array of chain linked list starts
	struct chain_link* critical_path;
	chains->c_l = malloc(sizeof(struct chain_link*));	// malloc space for realloc later
	chains->number_of_chains = 0;						// start with 0
	int i, j, k, max = 0;

	// find critical path
	for (i=0; i<n; i++) {								// iterate over every class
		if (courses[i].prereqs_num) {					// check if this course has prereqs
														// this just removes single courses (electives)
			int is_a_prereq = 0;						// reset 'is a prereq' flag
			for (j=0; j<n; j++) {						// iterate over classes again
				if (isPrereq(&(courses[i]), &(courses[j]))) {
					is_a_prereq = 1;					// set flag, this course is a prereq to another course
				}
			}

			if (!is_a_prereq) {
				// course is not a prereq, but has prereqs
				buildChain(chains, NULL, &(courses[i]), 0);
														// start a new chain, count starts at 0
														// first chain->next will be NULL
			}
		}
	}
	for (i=0; i<chains->number_of_chains; i++) {		// iterate over each chain link
		if (chains->c_l[i].count > max) {				// compare count and max
			critical_path = &(chains->c_l[i]);			// update critical path
			max = chains->c_l[i].count;					// update max
		}
	}

	printf("critical path found with length %d\n", max);

	// initalize variables
	struct course* chosen_one;
	struct semester* semesters;
														// pointer to start of linked list
	int season = sea;									// season variable
	int number_of_remaining_courses = n;				// to keep track of remaining courses not yet added
	int save = number_of_remaining_courses;				// this will be used to check if no courses were added

	// get number of remaining courses (uncredited)
	for (i=0; i<n; i++) {
		if (courses[i].status == 2)						// if we find a class already taken
			number_of_remaining_courses--;				// decrement number
	}

	// create the first semester (possible to streamline this later?)
	if (sea < 2) {										// check if summer or normal session
		semesters = createSemester(year, sea, m, NULL, NULL);
														// create normal semester
	} else {
		semesters = createSemester(year, sea, s, NULL, NULL);
														// create summer semester
	}

	// iterate over and over until courses are all contained, create semesters as needed
	while (number_of_remaining_courses > 0) {
		// if no classes were eligible, new semester
		if (save == number_of_remaining_courses) {
			for (i=0; i<semesters->number_of_courses; i++) {
														// iterate over courses in semester
				semesters->courses[i]->status = 2;		// set each class as taken
			}

			season++;									// increment season
			if (season > 2) {							// reset at 3 seasons
				season = 0;
				year++;
			}

			if (season < 2) {							// check if summer or normal session
				semesters = createSemester(year, season, m, NULL, semesters);
														// create normal semester
			} else {
				semesters = createSemester(year, season, s, NULL, semesters);
														// create summer semester
			}
		}

		// reset save
		save = number_of_remaining_courses;				// if this is the same value no courses were added

		// attempt to add prereq to next critical path item
		// FUTURE: recursive attempt to add each prereq of prereq etc.
		if (critical_path->next != NULL) {			// check that this isn't the end
			for (i=0; i<critical_path->next->c->prereqs_num; i++) {
													// iterate over next in crit path's prereqs
				chosen_one = critical_path->next->c->prereqs[i];
				switch (addCourseToSemester(semesters, chosen_one)) {
					case -1:
						printf(" error: pointer invalid!\n");
						return NULL;				// error, pointer was invalid
						break;
					case 0:
						number_of_remaining_courses--;
													// course added successfully, decrement number left
						break;
					case 1:
						break;						// course is not eligible
					case 2:
						break;						// not enough hours
				}
			}
		}

		// attempt to add critical path item
		if (critical_path != NULL) {					// check that critical path is still viable
			switch (addCourseToSemester(semesters, critical_path->c)) {
				case -1:
					printf(" error: pointer invalid!\n");
					return NULL;						// error, pointer was invalid
					break;
				case 0:
					number_of_remaining_courses--;		// course added successfully, decrement number left
					critical_path = critical_path->next;// head to next item on critical path
					break;
				case 1:
					break;								// course is not eligible
				case 2:
					break;								// not enough hours (should never happen)
			}
		}

		// attempt to add random class
		for (i=0; i<n; i++) {							// iterate over all remaining classes
			switch (addCourseToSemester(semesters, &(courses[i]))) {
				case -1:
					printf(" error: pointer invalid!\n");
					return NULL;						// error, pointer was invalid
					break;
				case 0:
					number_of_remaining_courses--;		// course added successfully, decrement number left
					break;
				case 1:
					break;								// course is not eligible
				case 2:
					break;								// not enough hours
			}
		}
	}

	// return pointer to semesters
	struct semester* temp;
	while (semesters->prev != NULL) {					// works backwards and sets next
		temp = semesters;								// save pointer
		semesters = semesters->prev;					// go back to start of semesters
		semesters->next = temp;							// restore pointer as next item
	}
	return semesters;									// return pointer to our plan
}
