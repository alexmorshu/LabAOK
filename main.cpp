#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>
#include <exception>
#include <cmath>

template<std::size_t N = 4>
class BigNum
{
	using number = std::uint16_t;
public:
	BigNum(const std::string& str): BigNum(new number[str.size()/N + 2]{}, str.size()/N + 2)
	{
		short minus = 0;
		if(str[0] == '-')
		{
			minus = 1;
		}
		number* buf = this->pointerToArr_();
		std::size_t position = 0;
		for(auto i = str.rbegin(); (i != str.rend()) && (*i != '-') ; i++, position++)
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
		if(minus == 1)
			this->invert();
	}


	BigNum(std::size_t capacity = 16) : BigNum(new number[capacity], capacity)
	{
		this->counter_() = 1;
		(*this->pointerToArr_()) = 0;	
	}

	BigNum(const BigNum& other) : BigNum(new number[other.capacity_], other.capacity_)
	{
		std::memcpy(this->arr_, other.arr_, other.capacity_*sizeof(number)); //copy
	}

	BigNum& operator=(const BigNum& other)
	{
		this->~BigNum();//destroy old

		number* newArr = new number[other.capacity_]; //allocate memomy
		std::memcpy(newArr, other.arr_, other.capacity_ * sizeof(number)); //copy
		this->arr_ = newArr; //set new arr
		return *this;
	}


	BigNum operator+(const BigNum& other) const
	{
		BigNum newNum (*this);
		newNum+=other;
		return newNum;
	}

	BigNum& add(std::uintmax_t num, bool otherSign)
	{
		if(otherSign)
		{
			if(num == 0)
				return *this;
			num--;
		}
		number thisSize = this->size();
                number thisCapacity = this->capacity_;
                number* thisBuf = this->pointerToArr_();
		bool thisSign = this->sign;
		std::size_t otherSize = (sizeof(std::uintmax_t))/N; 
		if(otherSize >= thisCapacity)
		{
			this->reallocate(otherSize + 1);
			thisBuf = this->pointerToArr_();
		}
		for(;thisSize < otherSize; thisSize++)
		{
			*(thisBuf + thisSize) = ((thisSign)?(BigNum::pow(N) - 1):(0) );
		}
		bool sign = false;
                for(number i = 0; i < thisSize; i++)
                {
                        number thisValue = *(thisBuf+i);
                        number otherValue = (num%BigNum::pow(N));
			if(otherSign)
				otherValue = (BigNum::pow(N)) - otherValue - 1;
			num/=BigNum::pow(N);
                        thisValue+=otherValue;
                        thisValue+=(sign)?(1):(0);
                        sign = thisValue >= BigNum::pow(N);
                        if(sign)
                        {
                                thisValue -= BigNum::pow(N);
                        }
                        *(thisBuf+i) = thisValue;
                }
		bool isOverflow = (thisSign && otherSign && !sign) ||
                                  (!thisSign && !otherSign && sign);

                bool nowSign = (thisSign && (otherSign || !sign)) || (otherSign && !sign);

                if(isOverflow)
                {
                        *(thisBuf+thisSize) = (thisSign)?(BigNum::pow(N)-2):(1);
                        thisSize++;
                }
                this->counter_() = thisSize;
                this->sign = nowSign;

		number g = (this->sign)?(BigNum::pow(N)-1):(0);
                for(number i = this->size()-1; i > 0 && (*(thisBuf+i) == g); i--)
                {
                        this->counter_()--;
                }
                return *this;
	}
	BigNum& operator+=(std::uintmax_t num)
	{
		return this->add(num, false);
	}
	BigNum& operator-=(std::uintmax_t num)
	{
		return this->add(num, true);
	}
	
	BigNum operator +(std::uintmax_t num) const
	{
		BigNum C (*this);
		C+=num;
		return C;
	}

	BigNum operator -(std::uintmax_t num) const
	{
		BigNum C (*this);
		C-=num;
		return C;
	}

	BigNum& operator-=(const BigNum& other)
	{
		this->invert();
		*this+=other;
		this->invert();
		return *this;
	}
	BigNum operator-(const BigNum& other)
	{
		BigNum C (*this);
		C-=other;
		return C;	
	}

	BigNum& operator+=(const BigNum& other)
	{
		number thisSize = this->size();
                number otherSize = other.size();
		number thisCapacity = this->capacity_;
		number* thisBuf = this->pointerToArr_();
		const number* otherBuf = other.pointerToArr_();

		bool thisSign = this->sign;
		bool otherSign = other.sign;	
		bool isSame = (thisSign && otherSign) || (!thisSign && !otherSign); 
		

		if(otherSize >= thisCapacity)
		{
			this->reallocate(otherSize + ((isSame)?(1):(0)));
			thisBuf = this->pointerToArr_();
		}
		for(;thisSize < otherSize; thisSize++)
		{
			*(thisBuf + thisSize) = ((thisSign)?(BigNum::pow(N) - 1):(0) );
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
		
		
		bool isOverflow = (thisSign && otherSign && !sign) ||
				  (!thisSign && !otherSign && sign);

		bool nowSign = (thisSign && (otherSign || !sign)) || (otherSign && !sign);
		if(isOverflow)
		{
			*(thisBuf+thisSize) = (thisSign)?(BigNum::pow(N)-2):(1);
			thisSize++;
		}
		this->counter_() = thisSize;
		this->sign = nowSign;
		
		number g = (this->sign)?(BigNum::pow(N)-1):(0);
		for(number i = this->size()-1; i > 0 && (*(thisBuf+i) == g); i--)
                {
                        this->counter_()--;
                }
		return *this;
		
	}

	int compare(const BigNum& other) const noexcept
	{
		bool thisSign = this->sign;
		bool otherSign = other.sign;
		if(!thisSign && otherSign)
			return -1;
		if(thisSign && !otherSign)
			return 1;

		number thisSize = this->size();
		number otherSize = other.size();
		if(thisSize > otherSize)
			return -1;
		else if(thisSize < otherSize)
			return 1;

		number* thisBuf = this->pointerToArr_();
		number* otherBuf = other.pointerToArr_();
		for(number i = 0; i < thisSize; i++)
		{
			number thisValue = *(thisBuf + (thisSize - 1) - i);
			number otherValue = *(otherBuf + (otherSize - 1) -i);
			if(thisValue > otherValue)
				return -1;

			else if (thisValue < otherValue)
				return 1;
		}
		return 0;
	}

	bool operator >(const BigNum& other) const noexcept
	{
		return -1 == this->compare(other);
	}


	bool operator < (const BigNum& other) const noexcept
	{
		return this->compare(other) == 1;
	}

	bool operator !=(const BigNum& other) const noexcept
	{
		return this->compare(other) != 0;
	}

	bool operator ==(const BigNum& other) const noexcept
	{
		return this->compare(other) == 0;
	}

	bool operator >=(const BigNum& other) const noexcept
	{
		return (*this > other) || (*this == other);
	}
	bool operator <=(const BigNum& other) const noexcept
	{
		return (*this < other) || (*this == other); 
	}
	
	void invert()
	{
		number* arr = this->pointerToArr_();
		bool sign = this->sign;
		if(*arr == 0 && this->size() == 0)
		{
			return;
		}
		if(!sign)
			*this -= 1;
	

		for(std::size_t i = 0; i < this->size(); i++)
		{	
			number* ptr = (arr+i);
			number value = *ptr;
			*ptr = BigNum::pow(N) - 1 - value;
			
		}
		if(sign)
			*this += 1;
		this->sign = !this->sign;
	}

	void debug()
	{
		std::cout << "sign: " << this->sign << ' ';
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
		short minus = ((this->sign)?(1):(0));
		std::string result(this->size()*N + minus, '0');
		
		if(minus == 1)
		{
			result[0] = '-';
			*this -= 1;
		}

		number size = this->size();
		number* buf = this->pointerToArr_();
		for(number i = 0; i < size; i++)
		{
			number num = *(buf + size - 1 - i);
		       	if(minus == 1)
			{
				num = (BigNum::pow(N) - 1) - num;
			}
			number O = BigNum::pow(N - 1); 
			for(int q = N-1; q >= 0 ; q-- )
			{
				number c = (num/O);
				num-=c*O;
				O/=10;
				result[i*N + N-1 - q + minus] = c + '0';
			}	
			
		}
		if(minus == 1)
		{
			*this += 1;
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
	bool sign = false;
};

BigNum<> operator ""_BN(const char* str)
{
	return BigNum(str);
}	

int main()
{
	BigNum A = 1111111111111111111111111111111111111111111111111111111111111111111111_BN;
	BigNum B = 4444444444444444444444444444444444444444444444444444444444444444444444_BN;
	BigNum C = A + B + 56789;
	
	std::cout << static_cast<std::string>(C) << std::endl;
}
