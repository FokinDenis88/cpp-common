#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>	// CustomException
//#include <exception> // CustomException
//#include <stdexcept> // different exception types
#include <functional> // for formatter function
#include <utility> // for move in Error class in set_info()


namespace general {

	namespace error {
//===========================Message Info Structures===============================

		/** Exception message information that always used in error messages */
		struct ErrorInfoGeneral {
			std::string source_file{};
			size_t		line_number{};
			std::string message{};
		};

		/** Detailed Exception information, that will be added to message. */
		struct ErrorInfoDetailed {
			ErrorInfoGeneral	general_info{};
			std::string			reason{};		// optional
			size_t				error_code{};	// optional
		};


		template<typename ErrorInfoType>
		constexpr bool IsEmptyErrorInfo(const ErrorInfoType& info) noexcept { // default variant
			return true;
		}
		template<>
		constexpr bool IsEmptyErrorInfo(const ErrorInfoGeneral& info) noexcept {
			if (!info.source_file.empty() && info.line_number != 0 && !info.message.empty()) {
				return false;
			}
			return true;
		}
		/** Is enough just to check GeneralErrorInfo */
		template<>
		constexpr bool IsEmptyErrorInfo(const ErrorInfoDetailed& info) noexcept {
			return IsEmptyErrorInfo(info.general_info);
		}


		/** Generate Message of format " Key+Separator+Value" f.e.: " Title: Description". */
		inline std::string GetKeySeparatorValue(const std::string& title,
												const std::string& description,
												const std::string& separator = ": ") noexcept {
			return title + separator + description;
		}

		/** Generate Error location message, that must show source file and line number. Maybe column. */
		inline std::string GetErrorLocation(const std::string& source_file, const size_t line_number) noexcept {
			if (source_file.empty() || 0 == line_number) { return ""; } // precondition

			return GetKeySeparatorValue(source_file, std::to_string(line_number), ":");
		}

//=====================Error Message Generator===============================
		// Errors Message Generators maybe be used, when you can't use exceptions in application.

		/** Generate an error message for CustomException. Default variant. */
		template<typename ErrorInfoType>
		inline std::string GenerateErrorMessage(const ErrorInfoType& info) { // default variant
			return "";
		};

		/** Specialization for generating an error message from a string. */
		template<>
		inline std::string GenerateErrorMessage(const std::string& info) {
			return GetKeySeparatorValue("Message", info);
		};

		/** Generate General Error message for CustomException. */
		template<>
		inline std::string GenerateErrorMessage(const ErrorInfoGeneral& info) {
			if (!IsEmptyErrorInfo(info)) { return ""; } // precondition

			std::string message = GetErrorLocation(info.source_file, info.line_number);
			message += ": " + GetKeySeparatorValue("Message", info.message);
			return message;
		};

		/** Generate Detailed Error message for CustomException. */
		template<>
		inline std::string GenerateErrorMessage(const ErrorInfoDetailed& info) {
			std::string message{ GenerateErrorMessage(info.general_info) };

			if (!info.reason.empty()) {	// optional
				message += " " + GetKeySeparatorValue("Reason", info.reason);
			}
			if (info.error_code != 0) { // optional
				message += " " + GetKeySeparatorValue("ErrorCode", std::to_string(info.error_code));
			}
			return message;
		};

//=====================Error class===============================================
		// TODO: refactor. IError maybe deleted?

		/* Abstract. Interface for errors. */
		class IError {
		protected:
			IError() = default;
			IError(const IError&) = delete; // C.67	C.21 Polymorphic suppress Copy & Move
			IError& operator=(const IError&) = delete;
			IError(IError&&) noexcept = delete;
			IError& operator=(IError&&) noexcept = delete;
		public:
			virtual ~IError() = default;


			virtual const std::string& GetMessage() const = 0;	// possible can be modified

			virtual void OutputToConsole() const = 0;

			virtual void Log() const = 0;

			virtual void Throw() const = 0;
		}; // !class IError


		/** Error class. Can be thrown or can be used in noexcept application */
		template<typename ErrorInfoType>
		class Error : public IError {
		public:
			/** Function object, that used to generate & format final error message. */
			using FormatterType = std::function<std::string(const ErrorInfoType&)>;

			/** Default generator & formatter of final error message. */
			using DefaultFormatterType = decltype([](const ErrorInfoType& info_p) { return GenerateErrorMessage(info_p); });


			Error() = default;
		protected:
			Error(const Error&) = delete; // C.67	C.21 Polymorphic suppress Copy & Move
			Error& operator=(const Error&) = delete;
			Error(Error&&) noexcept = delete;
			Error& operator=(Error&&) noexcept = delete;
		public:
			~Error() override = default;

			Error(const ErrorInfoType& info, FormatterType formatter = DefaultFormatterType{})
					:	error_info_{ info },
						formatter_{ formatter },
						output_message_{} {
			};
			Error(ErrorInfoType&& info, FormatterType formatter = DefaultFormatterType{})
					:	error_info_{ std::move(info) },
						formatter_{ std::move(formatter) },
						output_message_{} {
			};


			/** Return error message. */
			inline const std::string& GetMessage() const noexcept override {
				LazyMessageFormatting();
				return output_message_;
			}
			/** Return error message. Set new formatter. */
			virtual inline const std::string& GetMessage(FormatterType new_formatter = DefaultFormatterType{}) noexcept {
				formatter_ = new_formatter;
				output_message_ = new_formatter(error_info_);
				return output_message_;
			}
			/**
			* Return error message. Set new error information & formatter.
			*
			* @param new_info
			* @param formatter function object, that used to generate & format final error message.
			*/
			virtual inline const std::string& GetMessage(const ErrorInfoType& new_info,
														FormatterType new_formatter = DefaultFormatterType{}) noexcept {
				error_info_ = new_info;
				formatter_ = new_formatter;
				output_message_ = formatter_(new_info);
				return output_message_;
			};
			/**
			* Return error message. Set new error information by move & formatter.
			*
			* @param new_info
			* @param formatter function object, that used to generate & format final error message.
			*/
			virtual inline const std::string& GetMessage(ErrorInfoType&& new_info,
														FormatterType new_formatter = DefaultFormatterType{}) noexcept {
				error_info_ = std::move(new_info);
				formatter_ = new_formatter;
				output_message_ = formatter_(new_info);
				return output_message_;
			};


			/** Display error to error console. */
			inline void OutputToConsole() const noexcept override {
				//std::cerr << GetMessage() << "\n";
			}

			/** Write log message of error to file. */
			void Log() const override {
				/*std::ofstream logFile("error.log", std::ios_base::app);
				if (logFile.is_open()) {
					logFile << "[" << __TIME__ << "] Error code: " << GetCode()
						<< ", Message: " << GetMessage() << "\n";
					logFile.close();
				}*/
				/*std::ofstream logFile("error.log", std::ios_base::app);
				if (logFile.is_open()) {
					time_t now = std::time(nullptr);
					char buf[80];
					strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
					logFile << '[' << buf << "] Error: " << GetMessage() << '\n';
					logFile.close();
				}*/
			}

			/** Throw error as exception object */
			void Throw() const override {
				if (!IsEmptyErrorInfo(error_info_)) {
					//throw *this;
				}
			}

			// Method for convenient display of diagnostic information
			//#include <sstream>
			/*friend std::ostream& operator<<(std::ostream& os, const CustomException& ex) {
				os << ex.what();
				return os;
			}*/


			inline const ErrorInfoType& error_info() const noexcept { return error_info_; }
			inline const FormatterType& formatter() const noexcept { return formatter_; }

		private:
			inline void LazyMessageFormatting() const noexcept {
				if (output_message_.empty()) { output_message_ = formatter_(error_info_); }
			}

			/** Error information, that helps to format error message. */
			ErrorInfoType error_info_{};

			/** Define how information from error_info will be formatted in the output_message. */
			FormatterType formatter_{ DefaultFormatterType{} };

			/** Final message, that was generated from error_info. This message will be displayed. */
			mutable std::string output_message_{};	// mutable for lazy formatting on demand

		}; // !class Error

		using ErrorGeneral = Error<ErrorInfoGeneral>;
		using ErrorDetailed = Error<ErrorInfoDetailed>;

	} // !namespace error

} // !namespace general


#endif // !ERROR_HPP
