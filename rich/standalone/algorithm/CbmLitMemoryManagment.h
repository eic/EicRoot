#ifndef CBMLITMEMORYMANAGMENT_H_
#define CBMLITMEMORYMANAGMENT_H_


class DeleteObject {
public: 
	template<typename T>
	void operator()(const T* ptr) const {
		delete ptr;
	}	
};


#endif /*CBMLITMEMORYMANAGMENT_H_*/
