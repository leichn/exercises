template <typename T>
class deepcopy_smart_ptr
{
private:
    T* object;
public:
    //... other functions

    // copy constructor of the deepcopy pointer
    deepcopy_smart_ptr (const deepcopy_smart_ptr& source)
    {
        // Clone() is virtual: ensures deep copy of Derived class object
        object = source->Clone ();
    }

   // copy assignment operator
   deepcopy_smart_ptr& operator= (const deepcopy_smart_ptr& source)
   {
      if (object)
         delete object;

        object = source->Clone ();
   }
};
   
// stub to ensure compilation
int main()
{
   return 0;
}