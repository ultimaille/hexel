#define FOR(i, n) for(int i = 0; i < (int) n; i++)
#define plop(x) {std::cerr << "|plop|=>"<<" line:"<< __LINE__ <<"  "<< #x <<" : " <<x<< "     in  "<< __FILE__ <<std::endl;}

struct Log{
	inline static void add(std::string msg0,std::string msg1=""){ std::cerr<<"INFO: "     <<msg0<<" "<<msg1<<std::endl; }
	inline static void error(std::string msg0,std::string msg1=""){ std::cerr<<"ERROR: "    <<msg0<<" "<<msg1<<std::endl; }
	inline static void abort(std::string msg0,std::string msg1=""){ std::cerr<<"FATAL ERROR"<<msg0<<" "<<msg1<<std::endl; }
};

// Registry<T> is a lightweight named container for polymorphic objects (=> all stored types must inherit from T).
//
// The registry owns instances through std::unique_ptr<T>, so callers can
// manipulate them through the base interface without dealing with ownership.
// Each object is associated with a unique string name, which makes lookup by
// name convenient while preserving insertion order for iteration.

template<class T>
struct Registry {
    struct Element {
        std::string name;
        std::unique_ptr<T> object;
    };
    std::vector<Element> registry;

    template<class P, class... Args>
        P& emplace_back(std::string name, Args&&... args) {
            static_assert(std::is_base_of_v<T, P>);
            assert(!contains(name));

            auto object = std::make_unique<P>(
                    std::forward<Args>(args)...
                    );

            registry.push_back({
                    std::move(name),
                    std::move(object)
                    });

            return static_cast<P&>(*registry.back().object);
        }

    template<class... Args>
        T& emplace_back(std::string name, Args&&... args) {
            return emplace_back<T>(
                    std::move(name),
                    std::forward<Args>(args)...
                    );
        }

    bool contains(const std::string& name) const {
        return find(name) != -1;
    }

    int find(const std::string& name) const {
        for (int i = 0; i < static_cast<int>(registry.size()); ++i) {
            if (registry[i].name == name)
                return i;
        }
        return -1;
    }

    T& operator[](int index) {
        assert(index >= 0);
        assert(index < static_cast<int>(registry.size()));
        return *registry[index].object;
    }

    const T& operator[](int index) const {
        assert(index >= 0);
        assert(index < static_cast<int>(registry.size()));
        return *registry[index].object;
    }

    T& operator[](const std::string& name) {
        int index = find(name);
        assert(index >= 0);
        assert(index < static_cast<int>(registry.size()));
        return *registry[index].object;
    }

    const T& operator[](const std::string& name) const {
        int index = find(name);
        assert(index >= 0);
        assert(index < static_cast<int>(registry.size()));
        return *registry[index].object;
    }

    void pop_back() {
        assert(!registry.empty());
        registry.pop_back();
    }

    int size() const {
        return static_cast<int>(registry.size());
    }

    void erase(int index) {
        assert(index >= 0);
        assert(index < static_cast<int>(registry.size()));
        registry.erase(registry.begin() + index);
    }

    void erase(std::string &name) {
        int index = find(name);
        assert(index >= 0);
        assert(index < static_cast<int>(registry.size()));
        erase(index);
    }

    bool empty() const {
        return registry.empty();
    }
};


namespace TestAndDocForNamedCollections{
struct A{ void init(int i){ value = i; }	virtual int val(){ return value; }int value=0; };
struct B:public A{ virtual int val(){ return value+1; } };

	template<class Collection>
	void test_named_collections(Collection& collection){


		// WARNING: add only empty elements: A and B must have default constructors !
		B& b = collection.template emplace_back<B>("B");  // to add an element, we need to specify its class (derived)
		A& a = collection.emplace_back("A");		// elements of the root class don't need explicit type

		// constructors are replaced by init methods that can be directly called here
		collection.template emplace_back<B>("C").init(3);

		// acces to methods/members works as expected
		std::cerr<<"a value/val      "<< a.value<<"   "<<a.val()<<std::endl;
		std::cerr<<"b value/val      "<< b.value<<"   "<<b.val()<<std::endl;
		b.value=10;
		std::cerr<<"new b value/val  "<< b.value<<"   "<<b.val()<<std::endl;

		// acces by operator []
		std::cerr<<"acces by []  "<< collection["C"].value<<"   "<<collection["C"].val()<<std::endl;

		// can test and acces to data from their names
		if(collection.contains("A")) std::cerr<<" A exists "<<collection["A"].val()<<std::endl;
		if(collection.contains("D")) std::cerr<<" C exists "<<collection["C"].val()<<std::endl;
		collection.template emplace_back<B>("D").init(40);
		if(collection.contains("D")) std::cerr<<" C exists "<<collection["C"].val()<<std::endl;

		// can iterate on the collection
		for(auto &[name,obj]:collection.registry)	std::cerr<<"  ===  "<<name<<"  "<<obj->val(); std::cerr<<std::endl;

	};

	void test_named_collections(){
////	NamedMap<A>    amap; test_named_collections(amap);
////	plop(amap.has("A")); amap.remove("A"); plop(amap.has("A"));

		Registry<A> avec; test_named_collections(avec);
		// with the vector, we can use direct access for e.g. iterating directly on indices
		FOR(i,avec.size())	std::cerr<<"  ===  "<<i<<"  "<<avec[i].val(); std::cerr<<std::endl;
		avec.pop_back();
		FOR(i,avec.size())	std::cerr<<"  ===  "<<i<<"  "<<avec[i].val(); std::cerr<<std::endl;
	}
};
