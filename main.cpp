#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>
#include <exception>
template<std::size_t N = 4>
class BigNum
{
	using number = std::uint16_t;
public:
	BigNum(const std::string& str): BigNum(new number[str.size()/N + 2]{}, str.size()/N + 2)
	{
		number* buf = this->pointerToArr_();
		std::size_t position = 0;
		for(auto i = str.rbegin(); i != str.rend(); i++, position++)
		{
			std::size_t index = position/N;
			std::size_t num = position%N;
			if(!('0' <= (*i) && *i <= '9'))
				throw std::runtime_error("Not in base10");
			
			number result = *i - '0'; 
			for(std::size_t i = 0; i < num; i++)
			{
				result*=10;
			}
			*(buf+index) += result;
		}
		std::size_t index = str.size()/N;
		if(str.size()%N != 0)
		{
			index++;
		}
		this->counter_() = index;
	}


	BigNum(std::size_t capacity = 16) : BigNum(new number[capacity], capacity)
	{
		this->counter_() = 1;
		(*this->pointerToArr_()) = 0;	
	}

	BigNum(const BigNum& other) : BigNum(new number[other.capacity_], other.capacity_)
	{
		std::memcpy(this->arr_, other.arr_, other.capacity_*sizeof(std::uint16_t)); //copy
	}

	BigNum& operator=(const BigNum& other)
	{
		this->~BigNum();//destroy old

		number* newArr = new number[other.capacity_]; //allocate memomy
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
		const number count = other.size();
		const number thisCount = this->size(); 
		if(count > thisCount)
			throw std::runtime_error("This object is less than other");


	 	number* thisBuf = this->pointerToArr_();
	       	const number* otherBuf = other.pointerToArr_();	
		bool sign = false;
		for(number i = 0; i < count; i++)
		{
			number thisValue = *(thisBuf+i);
			const number otherValue = *(otherBuf+i);
			thisValue -= (sign)?(1):(0);
			sign = thisValue < otherValue;

			if(sign)
				thisValue = BigNum::pow(N) + thisValue - otherValue;
			else
				thisValue = thisValue - otherValue;

			*(thisBuf + i) = thisValue;
		}
		if(sign && (count == thisCount))
			throw std::runtime_error("This object is less than other");

		*(thisBuf+count)-=(sign)?(1):(0);

		for(number i = this->size()-1; i > 0 && (*(thisBuf+i) == 0); i--)
		{
			this->counter_()--;
		}
		return *this;
	}

	BigNum operator+(const BigNum& other) const
	{
		BigNum newNum (*this);
		newNum+=other;
		return newNum;
	}

	BigNum& operator+=(const BigNum& other)
	{
		number thisSize = this->size();
                number otherSize = other.size();
		number thisCapacity = this->capacity_;
		number* thisBuf = this->pointerToArr_();
		const number* otherBuf = other.pointerToArr_();
		if(otherSize >= thisCapacity)
		{
			this->reallocate(otherSize+1);
			thisBuf = this->pointerToArr_();
		}
		for(;thisSize < otherSize; thisSize++)
		{
			*(thisBuf + thisSize) = 0;
		}
		bool sign = false;
		for(number i = 0; i < thisSize; i++)
		{
			number thisValue = *(thisBuf+i);
			number otherValue = *(otherBuf+i);
			thisValue+=otherValue;
			thisValue+=(sign)?(1):(0);
			sign = thisValue >= BigNum::pow(N);
			if(sign)
			{
				thisValue -= BigNum::pow(N);
			}
			*(thisBuf+i) = thisValue;
		}

		if(sign)
		{
			*(thisBuf+thisSize) = 1;
			thisSize++;
		}
		this->counter_() = thisSize;
		return *this;
		
	}

	bool operator >(const BigNum& other) const noexcept
	{
		number thisSize = this->size();
		number otherSize = other.size();
		if(thisSize > otherSize)
			return true;
		else if(thisSize < otherSize)
			return false;

		number* thisBuf = this->pointerToArr_();
		number* otherBuf = other.pointerToArr_();
		for(number i = 0; i < thisSize; i++)
		{
			number thisValue = *(thisBuf + (thisSize - 1) - i);
			number otherValue = *(otherBuf + (otherSize - 1) -i);
			if(thisValue > otherValue)
				return true;

			else if (thisValue < otherValue)
				return false;
		}
		return false;
	}
	bool operator < (const BigNum& other) const noexcept
	{
		return other > (*this);
	}

	bool operator !=(const BigNum& other) const noexcept
	{
		return ((*this) > other) && ((*this) < other);
	}

	bool operator ==(const BigNum& other) const noexcept
	{
		return !((*this) != other);
	}

	bool operator >=(const BigNum& other) const noexcept
	{
		return (*this > other) || (*this == other);
	}
	bool operator <=(const BigNum& other) const noexcept
	{
		return (*this < other) || (*this == other); 
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
		std::memset(arr_, 0, this->capacity_ * sizeof(number)); //clear
		this->capacity_ = 0;
		delete[] std::launder(this->arr_);
	}

	number size() const noexcept
	{
		return *(this->arr_);
	}

	operator std::string()
	{
		std::string result(this->size()*N, '0');
		number size = this->size();
		number* buf = this->pointerToArr_();
		for(number i = 0; i < size; i++)
		{
			number num = *(buf + size - 1 - i); 
			number O = BigNum::pow(N - 1); 
			for(int q = N-1; q >= 0 ; q-- )
			{
				number c = (num/O);
				num-=c*O;
				O/=10;
				result.at(i*N + N-1 - q) = c + '0';
			}	
			
		}
		return result;
	}
private:
	BigNum(number* arr, std::size_t capacity): arr_(arr), capacity_(capacity)
	{}
	

	number& counter_() const noexcept
	{
		return *(this->arr_);
	}

	number* pointerToArr_() const noexcept
	{
		return this->arr_ + 1;
	}

	void reallocate(std::size_t size)
	{
		number* newArr = new number[size+1]{}; 
		number* old = this->arr_;
		std::memcpy(newArr, old, this->capacity_ * sizeof(number));
		delete[] old;
		this->arr_ = newArr;
		this->capacity_ = size+1;
	}
	
	static constexpr number pow(const number n)
	{
		number result = 1;
		for(std::size_t i = 0; i < n; i++ )
		{
			result *= 10;
		}
		return result;
	}
	
private:
	number* arr_;
	std::size_t capacity_;
};

 

int main()
{
	BigNum A("99994242423131331394444424242424244");
	BigNum B("14242424244242433424262476264672864");
	BigNum C = B - A;
	A.debug();
	std::cout << std::endl;
	B.debug();
	std::cout << std::endl;
	C.debug();
	std::cout << std::endl;
	std::cout << (A != A) << std::endl;
}
