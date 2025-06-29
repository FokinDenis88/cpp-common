#ifndef IEDITOR_HPP
#define IEDITOR_HPP

namespace common {

    /** Abstract. Interface for Editor class. CRUD operations */
    class IEditor {
    protected:
        IEditor() = default;
        IEditor(const IEditor&) = delete; // C.67	C.21
        IEditor& operator=(const IEditor&) = delete;
        IEditor(IEditor&&) noexcept = delete;
        IEditor& operator=(IEditor&&) noexcept = delete;
    public:
        virtual ~IEditor() = default;

    public:
        /** Adding new data. */
        virtual void Create() = 0;

        /** Retrieving existing data. */
        virtual void Read() = 0;

        /** Modifying data records. */
        virtual void Update() = 0;

        /** Removing data records. */
        virtual void Delete() = 0;
    };

} // !namespace common

#endif // !IEDITOR_HPP
