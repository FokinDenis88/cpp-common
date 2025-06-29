#ifndef CUSTOM_EXCEPTION_HPP
#define CUSTOM_EXCEPTION_HPP

#include <string>
#include <exception>	// CustomException
//#include <stdexcept>	// different exception types

#include "error.hpp"


namespace error {

	//=====================Exception Message Generator===============================
	// Maybe used, when exceptions are accepted by application. Exceptions are available.

	/**
	* Generate Base Exception inhereted from std::exception what message.
	*
	* @param base_what The original exception's `what()` string.
	* @return A formatted message with the key separator value
	*/
	inline std::string GenerateBaseWhat(const char* base_what) noexcept {
		if (base_what && *base_what != '\0') {
			return GetKeySeparatorValue("Base Exception", base_what);
		}
		return {};
	};

	/**
	 * Generate Error message for CustomException.
	 * To ErrorT message add base_what message.
	 */
	template<typename ErrorT>
	inline std::string GenerateExceptionMessage(const ErrorT& error,
		const char* base_what = '\0') {
		std::string message{ error.GetMessage() };
		if (base_what) {
			message += "\n";
			message += GenerateBaseWhat(base_what);
		}
		return message;
	};

	//========================Custom Exception Class===============================
	// TODO: make CustomException more flexible

	/**
	* Represents a generic custom exception derived from std::exception.
	*
	* This allows embedding additional context-specific error data into the exception.
	*
	* @tparam ErrorT The specific error type holding extra details about the failure.
	*/
	template<typename ExceptionT = std::exception, typename ErrorInfoT = ErrorInfoGeneral>
	// requires std::is_base_of_v<> derived from Error | requires std::is_base_of_v<exception>
	class CustomException : public ExceptionT {
	public:
		using ErrorT = Error<ErrorInfoT>;

		/** Function object, that used to generate & format final error message. */
		using FormatterT = typename ErrorT::FormatterT;

		/** Default generator & formatter of final error message. */
		static inline const FormatterT DefaultFormatter{ ErrorT::DefaultFormatter };

	protected:
		CustomException(const CustomException&) = delete; // polymorphic class suppress copy/move C.67
		CustomException& operator=(const CustomException&) = delete;
		CustomException(CustomException&&) noexcept = delete;
		CustomException& operator=(CustomException&&) noexcept = delete;
	public:
		~CustomException() override = default;


		CustomException(const ExceptionT& exception, const ErrorT& error)
			: ExceptionT{ exception }, // Base class initialization
			error_{ error } {
		}
		CustomException(const ExceptionT& exception,
			const ErrorInfoT& info,
			FormatterT formatter = DefaultFormatter)
			: ExceptionT{ exception }, // Base class initialization
			error_{ info, std::move(formatter) } {
		}
		template<typename ErrorInfoT>
		CustomException(const ExceptionT& exception,
			ErrorInfoT&& info,
			FormatterT formatter = DefaultFormatter)
			: ExceptionT{ exception }, // Base class initialization
			error_{ std::forward<ErrorInfoT>(info), std::move(formatter) } {
		}


		const char* what() const noexcept override {
			LazyWhatFormatting();
			return what_.c_str();
		}


		// Method for convenient display of diagnostic information
		//#include <sstream>
		/*friend std::ostream& operator<<(std::ostream& os, const CustomException& ex) {
			os << ex.what();
			return os;
		}*/


		/** Get base exception's what message. */
		inline const char* base_what() const noexcept {
			return this->ExceptionT::what();
		}
		/** Getter for error object stored in exception. */
		inline const ErrorT& get_error() const noexcept { return error_; }

	private:
		// Basic exception message is inside ExceptionT

		inline void LazyWhatFormatting() const noexcept {
			if (what_.empty()) {
				what_ = GenerateExceptionMessage(error_, this->std::exception::what());
			}
		}

		/**
		 * Additional Information, that helps to generate what_ error message.
		 * And better describes basic exception.
		 */
		ErrorT error_{};

		/** Final error message that will be displayed to user. */
		mutable std::string what_{};

	}; // !class CustomException

	template<typename ExceptionT = std::exception>
	using GeneralException = CustomException<ExceptionT, ErrorInfoGeneral>;

	template<typename ExceptionT = std::exception>
	using CodeException = CustomException<ExceptionT, ErrorInfoCode>;

	template<typename ExceptionT = std::exception>
	using DetailedException = CustomException<ExceptionT, ErrorInfoDetailed>;


#define FILE_N_LINE __FILE__, __LINE__
	//#define THROW_DETAILED_EXCEPTION(msg, reason, error_code) \
	//		throw CustomException({__FILE__, __LINE__, msg, reason, error_code});


	//// Использование макросов
	//#define LOG_AND_THROW_CUSTOM_EXCEPTION(msg, reason) \
	//    do {                                           \
	//        std::cerr << "Error occurred: " << msg << std::endl; \
	//        THROW_CUSTOM_EXCEPTION(msg, reason);       \
	//    } while(false)


	/** Just snippet for copy past. */
	class ConcreteException : public CodeException<std::system_error> {
	public:
		using ConcreteExceptionErrorT = CodeException<std::system_error>;
		using FormatterT = ConcreteExceptionErrorT::FormatterT;
		using ConcreteExceptionErrorT::DefaultFormatter;

	protected:
		ConcreteException(const ConcreteException&) = delete; // polymorphic class suppress copy/move C.67
		ConcreteException& operator=(const ConcreteException&) = delete;
		ConcreteException(ConcreteException&&) noexcept = delete;
		ConcreteException& operator=(ConcreteException&&) noexcept = delete;
	public:
		~ConcreteException() override = default;


		ConcreteException(const std::system_error& exception,
			const ErrorInfoCode& info,
			FormatterT formatter = DefaultFormatter)
			: ConcreteExceptionErrorT{ exception, info, std::move(formatter) } { // move formatter, cause resources maybe in fn
		}
		ConcreteException(const std::system_error& exception,
			ErrorInfoCode&& info,
			FormatterT formatter = DefaultFormatter)
			: ConcreteExceptionErrorT{ exception, std::forward<ErrorInfoCode>(info), std::move(formatter) } {
		}

	}; // !class ConcreteException

} // !namespace error


#endif // !CUSTOM_EXCEPTION_HPP
