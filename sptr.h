template <class T>
class UniquePtr {
public:
    UniquePtr() {
		m_ptr = nullptr;
	}
    UniquePtr(const UniquePtr& o) = delete;
    UniquePtr& operator=(const UniquePtr& o) = delete;
    
    UniquePtr(UniquePtr&& o) noexcept {
		m_ptr = o.m_ptr;
		o.m_ptr = nullptr;
	}
    
	UniquePtr& operator=(UniquePtr&& o) noexcept { 
		if (m_ptr != nullptr)
		{
			delete m_ptr;
		}
		m_ptr = o.m_ptr;
		o.m_ptr = nullptr;
		return *this;
	}
    
	UniquePtr(T* p) {
		m_ptr = p;
	}
    
	T* operator->() { 
		return m_ptr; 
	}
    
    T& operator*() { 
		return *m_ptr;
	}
    
	~UniquePtr() {
		delete m_ptr;
		m_ptr = nullptr;
	}
    
private:
	T* m_ptr;
};


// ================================================
struct RefCntBlock {
    size_t strong, weak;
};

template <class T>
class WeakPtr;

template <class T>
class SharedPtr {   
    friend class WeakPtr<T>;
public:
    SharedPtr() {
		m_ptr = nullptr;
		m_ref_block = nullptr;
	}
	SharedPtr(const SharedPtr& o){
		if (o.m_ptr == nullptr)
		{
			SharedPtr();
		}
		else
		{
			m_ptr = o.m_ptr;
			m_ref_block = o.m_ref_block;
			++m_ref_block->strong;
		}
	}
    SharedPtr& operator=(const SharedPtr& o) {
		if (&o == this)
		{
			return *this;
		}
		Reset();
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		if (m_ptr != nullptr)
		{
			++m_ref_block->strong;
		}
		return *this;
	}
    
    SharedPtr(SharedPtr&& o) noexcept { 
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		o.m_ptr = nullptr;
		o.m_ref_block = nullptr;
	} 
    
    SharedPtr& operator=(SharedPtr&& o) {
		Reset();
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		o.m_ptr = nullptr;
		o.m_ref_block = nullptr;
		return *this;
	}
    
    explicit SharedPtr(T* p) {
		m_ptr = p;
		m_ref_block = nullptr;
		if (m_ptr != nullptr)
		{
			m_ref_block = new RefCntBlock;
			m_ref_block->strong = 1;
			m_ref_block->weak = 0;
		}
	}
    
    // Implementation below
    explicit SharedPtr(const WeakPtr<T>& o);
    
    // Replaces pointer with nullptr
    void Reset() {
		if (m_ptr != nullptr)
		{
			if (m_ref_block->strong == 1)
			{
				delete m_ptr;
				m_ref_block->strong = 0;
				if (m_ref_block->weak == 0)
				{
					delete m_ref_block;
				}
			}
			else
			{
				--m_ref_block->strong;
			}
			m_ptr = nullptr;
			m_ref_block = nullptr;
		}
	}
    
    T* operator->() {
		return m_ptr;
	}
    
    T& operator*() {
		return *m_ptr;
	}
    
	~SharedPtr() { 
		Reset(); 
	}
    
private:
	T* m_ptr;
	RefCntBlock* m_ref_block;
};

template <class T>
class WeakPtr {
    friend class SharedPtr<T>;
public:
    WeakPtr() {
		m_ptr = nullptr;
		m_ref_block = nullptr;
	}
    WeakPtr(const WeakPtr& o) {
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		if (m_ptr != nullptr)
		{
			++m_ref_block->weak;
		}
    }
    WeakPtr& operator=(const WeakPtr& o) {
		if (this == &o)
		{
			return *this;
		}
		Reset();
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		if (m_ptr != nullptr)
		{
			++m_ref_block->weak;
		}
		return *this;
	}
    
    WeakPtr(WeakPtr&& o) {
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		o.m_ptr = nullptr;
		o.m_ref_block = nullptr;
	}
    WeakPtr& operator=(WeakPtr&& o) { 
		Reset();
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		o.m_ptr = nullptr;
		o.m_ref_block = nullptr;
		return *this;
	}
    
    WeakPtr(const SharedPtr<T>& o) { 
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		if (m_ptr != nullptr)
		{
			++m_ref_block->weak;
		}
	}
	WeakPtr& operator=(const SharedPtr<T>& o) {
		Reset();
		m_ptr = o.m_ptr;
		m_ref_block = o.m_ref_block;
		if (m_ptr != nullptr)
		{
			++m_ref_block->weak;
		}
		return *this;
	}
    
    // Replaces pointer with nullptr
    void Reset() {
		if (m_ptr != nullptr)
		{
			if (m_ref_block->weak == 1 && m_ref_block->strong == 0)
			{
				delete m_ref_block;
			}
			else
			{
				--m_ref_block->weak;
			}
		}
		m_ptr = nullptr;
		m_ref_block = nullptr;
	}
    
    bool Expired() const { 
		if (m_ptr == nullptr)
		{
			return true;
		}
		if (m_ref_block->strong == 0)
		{
			return true;
		}
		return false;
	}
    
    SharedPtr<T> Lock() {  
		return SharedPtr<T>(*this);
	}
        
    ~WeakPtr () { 
		Reset();
	}
    
private:
	T* m_ptr;
	RefCntBlock* m_ref_block;
};

template <class T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& o) {
	m_ptr = o.m_ptr;
	m_ref_block = o.m_ref_block;
	if (m_ptr != nullptr)
	{
		++m_ref_block->strong;
	}
}
