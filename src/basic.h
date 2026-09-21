#define FOR(i, n) for(int i = 0; i < (int) n; i++)
#define plop(x) {std::cerr << "|plop|=>"<<" line:"<< __LINE__ <<"  "<< #x <<" : " <<x<< "     in  "<< __FILE__ <<std::endl;}

struct Log{
	inline static void add(std::string msg0,std::string msg1=""){ std::cerr<<"INFO: "     <<msg0<<" "<<msg1<<std::endl; }
	inline static void error(std::string msg0,std::string msg1=""){ std::cerr<<"ERROR: "    <<msg0<<" "<<msg1<<std::endl; }
	inline static void abort(std::string msg0,std::string msg1=""){ std::cerr<<"FATAL ERROR"<<msg0<<" "<<msg1<<std::endl; }
};

// -------------------------------------------------------------------------------
//                                    Named Collections
// -------------------------------------------------------------------------------
// => objects are too big and polymorph for storing them directly in the structure
// => we can use unique_ptr with an interface using only references (no pointers for the users)
// 



/*
template<class T>
struct NamedMap {
	std::map<std::string,std::unique_ptr<T>> collection;

	template<class P> P& add(std::string name){
		um_assert(!has(name));
		std::unique_ptr<T>& ptr =  collection[name];
		ptr  =std::make_unique<P>();
		return static_cast<P&>(*ptr);
	}

	T& add(std::string name){ return add<T>(name); }

	bool has(std::string name){ return collection.find(name)!=collection.end(); }

	void remove(std::string name){ collection.erase(name); }

	T& operator[](std::string name){
		auto it = collection.find(name);
		um_assert(it!=collection.end());
		T& ptr = *(it->second.get());
		return ptr;
	}
};
*/

template<class T>
struct NamedVector {
	typedef std::pair<std::string,std::unique_ptr<T>> elt;
	std::vector<elt> collection;

	template<class P> P& add(std::string name){
		um_assert(!has(name));
		collection.push_back({name,std::make_unique<P>()});
		return static_cast<P&>(*collection.back().second.get());
	}

	T& add(std::string name){ return add<T>(name); }

	int id(std::string name){
		FOR(i,collection.size())
			if(collection[i].first.compare(name)==0)
				return i;
		return -1;
	}
	bool has(std::string name){
		int i = id(name);
		return (i!=-1);
	}
	T& operator[](int i){
		return *collection[i].second.get();
	}
	T& operator[](std::string name){
		int i = id(name);
		um_assert(i!=-1);
		return *collection[i].second.get();
	}
	void pop_back(){ collection.pop_back(); }
	int size(){ return collection.size(); }
};


namespace TestAndDocForNamedCollections{
struct A{ void init(int i){ value = i; }	virtual int val(){ return value; }int value=0; };
struct B:public A{ virtual int val(){ return value+1; } };

	template<class Collection>
	void test_named_collections(Collection& collection){


		// WARNING: add only empty elements: A and B must have default constructors !
		B& b = collection.template add<B>("B");  // to add an element, we need to specify its class (derived)
		A& a = collection.add("A");		// elements of the root class don't need explicit type

		// constructors are replaced by init methods that can be directly called here
		collection.template add<B>("C").init(3);

		// acces to methods/members works as expected
		std::cerr<<"a value/val      "<< a.value<<"   "<<a.val()<<std::endl;
		std::cerr<<"b value/val      "<< b.value<<"   "<<b.val()<<std::endl;
		b.value=10;
		std::cerr<<"new b value/val  "<< b.value<<"   "<<b.val()<<std::endl;

		// acces by operator []
		std::cerr<<"acces by []  "<< collection["C"].value<<"   "<<collection["C"].val()<<std::endl;

		// can test and acces to data from their names
		if(collection.has("A")) std::cerr<<" A exists "<<collection["A"].val()<<std::endl;
		if(collection.has("D")) std::cerr<<" C exists "<<collection["C"].val()<<std::endl;
		collection.template add<B>("D").init(40);
		if(collection.has("D")) std::cerr<<" C exists "<<collection["C"].val()<<std::endl;

		// can iterate on the collection
		for(auto &[name,obj]:collection.collection)	std::cerr<<"  ===  "<<name<<"  "<<obj->val(); std::cerr<<std::endl;

	};

	void test_named_collections(){
////	NamedMap<A>    amap; test_named_collections(amap);
////	plop(amap.has("A")); amap.remove("A"); plop(amap.has("A"));

		NamedVector<A> avec; test_named_collections(avec);
		// with the vector, we can use direct access for e.g. iterating directly on indices
		FOR(i,avec.size())	std::cerr<<"  ===  "<<i<<"  "<<avec[i].val(); std::cerr<<std::endl;
		avec.pop_back();
		FOR(i,avec.size())	std::cerr<<"  ===  "<<i<<"  "<<avec[i].val(); std::cerr<<std::endl;
	}
};
