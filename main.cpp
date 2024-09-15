#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>
#include <exception>
class BigNum
{
public:
	BigNum(const std::string& str): BigNum(new std::uint16_t[str.size()/4 + 2]{}, str.size()/4 + 2)
	{
		std::uint16_t size = 0;
		std::uint16_t* buf = this->pointerToArr_();
		std::size_t position = 0;
		for(auto i = str.rbegin(); i != str.rend(); i++, position++)
		{
			std::size_t index = position/4;
			std::size_t num = position%4;
			if(!('0' <= (*i) && *i <= '9'))
				throw std::runtime_error("Not in base10");
			
			std::uint16_t result = *i - '0'; 
			for(std::uint8_t i = 0; i < num; i++)
			{
				result*=10;
			}
			*(buf+index) += result;
		}
		std::uint16_t index = str.size()/4;
		if(str.size()%4 != 0)
		{
			index++;
		}
		this->counter_() = index;	
	}


	BigNum(std::size_t capacity = 16) : BigNum(new std::uint16_t[capacity]{}, capacity)
	{
		this->counter_() = 1;
		(*this->pointerToArr_()) = 0;	
	}

	BigNum(const BigNum& other) : BigNum(new std::uint16_t[other.capacity_], other.capacity_)
	{
		std::memcpy(this->arr_, other.arr_, other.capacity_*sizeof(std::uint16_t)); //copy
	}

	BigNum& operator=(const BigNum& other)
	{
		this->~BigNum();//destroy old

		std::uint16_t* newArr = new std::uint16_t[other.capacity_]; //allocate memomy
		std::memcpy(newArr, other.arr_, other.capacity_ * sizeof(std::uint16_t)); //copy
		this->arr_ = newArr; //set new arr
		return *this;
	}

	BigNum operator-(const BigNum& other) const
	{
		BigNum newNum (*this);
		newNum-=other;
		return newNum;
	}

	BigNum& operator-=(const BigNum& other)
	{
		const std::uint16_t count = other.size(); 
	 	std::uint16_t* thisBuf = this->pointerToArr_();
	       	const std::uint16_t* otherBuf = other.pointerToArr_();	
		bool sign = false;
		for(std::uint16_t i = 0; i < count; i++)
		{
			std::uint16_t thisValue = *(thisBuf+i);
			const std::uint16_t otherValue = *(otherBuf+i);
			thisValue -= (sign)?(1):(0);
			sign = thisValue < otherValue;

			if(sign)
				thisValue = BigNum::O_ + thisValue - otherValue;
			else
				thisValue = thisValue - otherValue;

			*(thisBuf + i) = thisValue;
		}
		*(thisBuf+count)-=(sign)?(1):(0);

		for(std::uint16_t i = this->size()-1; i > 0 && (*(thisBuf+i) == 0); i--)
		{
			this->counter_()--;
		}
		return *this;
	}

	bool operator >(const BigNum& other)
	{
		std::uint16_t thisSize = this->size();
		std::uint16_t otherSize = other.size();
		if(thisSize > otherSize)
			return true;
		else if(thisSize < otherSize)
			return false;

		std::uint16_t* thisBuf = this->pointerToArr_();
		std::uint16_t* otherBuf = other.pointerToArr_();
		for(std::uint16_t i = 0; i < thisSize; i++)
		{
			std::uint16_t thisValue = *(thisBuf + (thisSize - 1) - i);
			std::uint16_t otherValue = *(otherBuf + (otherSize - 1) -i);
			if(thisValue > otherValue)
				return true;

			else if (thisValue < otherValue)
				return false;
		}
		return false;
	}

	void debug()
	{
		for(int i = 0; i < this->counter_(); i++)
		{
			std::cout << *(this->pointerToArr_()+i) << ' ';
		}
	}

	~BigNum()
	{
		std::memset(arr_, 0, this->capacity_ * sizeof(std::uint16_t)); //clear
		this->capacity_ = 0;
		delete[] this->arr_;
	}

	std::uint16_t size() const noexcept
	{
		return *(this->arr_);
	}

	operator std::string()
	{
		std::string result(this->size()*4, '0');
		std::uint16_t size = this->size();
		std::uint16_t* buf = this->pointerToArr_();
		for(std::uint16_t i = 0; i < size; i++)
		{
			std::uint16_t num = *(buf + size - 1 - i); 
			std::size_t O = BigNum::O_/10; 
			for(int q = 3; q >= 0 ; q-- )
			{
				std::uint16_t c = (num/O);
				num-=c*O;
				O/=10;
				result.at(i*4 + 3 - q) = c + '0';
			}	
			
		}
		return result;
	}

private:
	BigNum(std::uint16_t* arr, std::size_t capacity): arr_(arr), capacity_(capacity)
	{}
	

	std::uint16_t& counter_() const noexcept
	{
		return *(this->arr_);
	}

	std::uint16_t* pointerToArr_() const noexcept
	{
		return this->arr_ + 1;
	}

	void reallocate(std::size_t add = 10)
	{
		std::size_t newCapacity = this->capacity_ + add;
		std::uint16_t* newArr = new uint16_t[this->capacity_ + add];
		std::memcpy(newArr, this->arr_, this->capacity_ * sizeof(std::uint16_t));
		delete[] this->arr_;
		this->arr_ = newArr;
		this->capacity_ = newCapacity;
	}

	
private:
	std::uint16_t* arr_;
	std::size_t capacity_;
	static const std::uint16_t O_ = 10000;
};



int main()
{
	BigNum A("8278473877837858937895789375897389578937589738975893758973895789375897389587362964785278582486264879276948727854");
	BigNum B("3410151683789593856926894729872875482798168735197489628548248972646275872649728467826498728642649828472987489628");
	if(A > B){
		BigNum C = A-B;
		std::cout << static_cast<std::string> (C) << std::endl;
	}
}
