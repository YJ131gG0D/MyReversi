#include <chrono>
#include <sstream>

#include "cpptcl.h"
#include "reversi_tcl.h"

using namespace Tcl;

game_gui mygame;
tree book;
interpreter* ptr_inter;
bool flag_echo = false;

void game_gui::print_log(const string& str){
	return ::print_log(str);
}

object brd2obj(cboard brd){
	object result;
	result.append(*ptr_inter,object((int)(brd.bget(true) >> 32)));
	result.append(*ptr_inter,object((int)(brd.bget(true))));
	result.append(*ptr_inter,object((int)(brd.bget(false) >> 32)));
	result.append(*ptr_inter,object((int)(brd.bget(false))));
	return result;
}

board obj2brd(const object& obj){
	board brd;
	brd_type brd_black,brd_white;
	brd_black = (unsigned int)obj.at(*ptr_inter,0).get<int>(*ptr_inter);
	brd_black <<= 32;
	brd_black |= (unsigned int)obj.at(*ptr_inter,1).get<int>(*ptr_inter);
	brd_white = (unsigned int)obj.at(*ptr_inter,2).get<int>(*ptr_inter);
	brd_white <<= 32;
	brd_white |= (unsigned int)obj.at(*ptr_inter,3).get<int>(*ptr_inter);
	brd.assign(brd_black,brd_white);
	return brd;
}

object choice2obj(cchoice c){
	object result;
	result.append(*ptr_inter,object(double(c.val)));
	result.append(*ptr_inter,object(int(c.pos & 7)));
	result.append(*ptr_inter,object(int(c.pos >> 3)));
	return result;
}

choice obj2choice(const object& obj){
	choice result;
	result.val = obj.at(*ptr_inter,0).get<double>(*ptr_inter);
	result.pos = obj.at(*ptr_inter,1).get<int>(*ptr_inter)
		+ (obj.at(*ptr_inter,2).get<int>(*ptr_inter) << 3);
	return result;
}

template<typename T>
object vec2obj(const vector<T>& vec){
		object result;
		for(const T& i:vec){
			result.append(*ptr_inter,object(i));
		}
		return result;
}

template<>
object vec2obj(const vector<object>& vec){
		object result;
		for(const object& i:vec){
			result.append(*ptr_inter,i);
		}
		return result;
}

vector<object> obj2vec(const object& objs){
		vector<object> result;
		object obj;
		int num = objs.length(*ptr_inter);
		for(int i = 0;i != num;++i){
			obj = objs.at(*ptr_inter,i);
			result.push_back(obj);
		}
		return result;
}

void start(){
	mygame.start();
}

void auto_save(cbool flag){
	mygame.flag_auto_save = flag;
}

object bget(){
	return brd2obj(mygame.brd);
}

void assign(const object& obj){
	board brd = obj2brd(obj);
	mygame.assign(brd);
}

int visit(cint x, cint y){
	return mygame.get(pos_type(x),pos_type(y));
}

void set(cint x, cint y, cint chsman){
	return mygame.set(pos_type(x),pos_type(y),chessman(chsman));
}

int count(cbool color){
	return mygame.count(color);
}

int count_move(cbool color){
	return mygame.count_move(color);
}

float score(cbool color){
	return mygame.score(color);
}

void config(){
	pattern::config();
	mygame.config();
	grp.load("data/pattern.dat");
} 
bool flip(cbool color,cint x,cint y){
	return mygame.flip(color,x,y);
}

object play(cint mthd,cbool color,cint height){
	auto pos = mygame.play(method(mthd),color,height);
	object result;
	result.append(*ptr_inter,object(int(pos.x)));
	result.append(*ptr_inter,object(int(pos.y)));
	return result;
}

object plays(cint x,cint y,cint mthd,cint height){
	auto pos = mygame.play(coordinate(x,y),method(mthd),height);
	object result;
	result.append(*ptr_inter,object(int(pos.x)));
	result.append(*ptr_inter,object(int(pos.y)));
	return result;
}

bool undo(){
	return mygame.undo();
}
bool redo(){
	return mygame.redo();
}

void mirror_h(){
	return mygame.mirror_h();
}
void mirror_v(){
	return mygame.mirror_v();
}
void reflect(){
	return mygame.reflect();
}
void rotate_l(){
	return mygame.rotate_l();
}
void rotate_r(){
	return mygame.rotate_r();
}
void reverse(){
	return mygame.reverse();
}

bool get_color(){
	return mygame.color;
}
void set_color(cbool color){
	mygame.set_color(color);
}

int get_method(){
	return mygame.mthd;
}
void set_method(cint mthd){
	mygame.mthd = method(mthd);
}

int get_depth(){
	return mygame.depth;
}
void set_depth(cint depth){
	mygame.depth = depth;
}

bool get_is_lock(){
	return mygame.flag_lock;
}
void set_is_lock(cbool flag_lock){
	mygame.flag_lock = flag_lock;
}

object get_pos(){
	object result;
	result.append(*ptr_inter,object(int(mygame.pos.x)));
	result.append(*ptr_inter,object(int(mygame.pos.y)));
	return result;
}
void set_pos(cint x,cint y){
	mygame.set_pos(x,y);
}

object get_choice(cint mthd,cbool color,cint height){
	vector<choice> choices = mygame.get_choice(method(mthd),color,height);
	show_choice(choices);
	vector<object> vec;
	for(const choice& c:choices){
		vec.push_back(choice2obj(c));
	}
	return vec2obj(vec);
}

object select_choice(object objs){
	vector<object> vec = obj2vec(objs);
	vector<choice> choices;
	for(const object& obj:vec){
		choices.push_back(obj2choice(obj));
	}
	return choice2obj(mygame.select_choice(choices));
}

void load(const string& path){
	return book.load(path);
};

void grp_initial(cint size){
	return grp.initial(size);
}
void grp_load(const string& filename){
	return grp.load(filename);
}
void grp_save(const string& filename){
	return grp.save(filename);
}

void process(const string& str){

	static interpreter inter;
	static bool flag = true;

	if(flag){
		flag = false;
		ptr_inter = &inter;

		inter.def("quit",::quit);
		inter.def("exit",::quit);
		inter.def("puts",::print_term);
		inter.def("print",::print_term);

		inter.def("start",::start);
		inter.def("config",::config);
		inter.def("flip",::flip);
		inter.def("play",::play);
		inter.def("plays",::plays);
		inter.def("undo",::undo);
		inter.def("redo",::redo);
		inter.def("mirror_h",::mirror_h);
		inter.def("mirror_v",::mirror_v);
		inter.def("reflect",::reflect);
		inter.def("rotate_l",::rotate_l);
		inter.def("rotate_r",::rotate_r);
		inter.def("reverse",::reverse);
		inter.def("auto_save",::auto_save);
		inter.def("bget",::bget);
		inter.def("assign",::assign);
		inter.def("visit",::visit);
		inter.def("place",::set);
		inter.def("get_color",::get_color);
		inter.def("set_color",::set_color);
		inter.def("get_method",::get_method);
		inter.def("set_method",::set_method);
		inter.def("get_depth",::get_depth);
		inter.def("set_depth",::set_depth);
		inter.def("get_pos",::get_pos);
		inter.def("set_pos",::set_pos);
		inter.def("get_is_lock",::get_is_lock);
		inter.def("set_is_lock",::set_is_lock);
		inter.def("get_choice",::get_choice);
		inter.def("select_choice",::select_choice);
		inter.def("count",::count);
		inter.def("count_move",::count_move);
		inter.def("score",::score);

		inter.def("load",::load);

		inter.def("grp_initial",::grp_initial);
		inter.def("grp_load",::grp_load);
		inter.def("grp_save",::grp_save);

		inter.def("load_book",::load_book);

		inter.eval(
			"set blank 0;"
			"set white 1;"
			"set black 2;"
			"set null 3;"

			"set false 0;"
			"set true 1;"

			"set mthd_rnd 0x0;"
			"set mthd_ab 0x1;"
			"set mthd_kill 0x2;"
			"set mthd_pvs 0x4;"
			"set mthd_trans 0x8;"
			"set mthd_mtdf 0x10;"
			"set mthd_ids 0x20;"
			"set mthd_ptn 0x40;"
			"set mthd_mpc 0x80;"
			"set mthd_end 0x100;"

			"config;"
		);
	}

	if(flag_echo){
		print_term(string(">>") + str);
	}

	try{
		chrono::system_clock::time_point time_start = chrono::system_clock::now();
		inter.eval(str);
		chrono::system_clock::time_point time_end = chrono::system_clock::now();
		chrono::duration<double> time_exec = time_end - time_start;
		print_status(
			"Execution time : " + to_string(time_exec.count()) + " seconds"
		);
	}catch(const tcl_error& err){
		print_term(string(err.what()));
	}
}