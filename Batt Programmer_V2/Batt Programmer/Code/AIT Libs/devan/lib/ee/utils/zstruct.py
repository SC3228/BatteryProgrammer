"""This recipe provides a higher level wrapper around the struct module. It provides a more 
convenient syntax for defining and using structs, and adds additional features such as: 
- Allows embedding structures within other structures 
- Allows defining arrays of items (or other structures) 
- Class based syntax, allowing access and updates by field name, not position 
- Extension of structures by inheritance

copied from http://code.activestate.com/recipes/498149-a-higher-level-struct-module

Example code can be found in the above URL as well.

"""
# Built-in
import struct

# Six Imports
import six


class Format(object):
    """Endianness and size format for structures."""
    Native          = "@"       # Native format, native size
    StandardNative  = "="       # Native format, standard size
    LittleEndian    = "<"       # Standard size
    BigEndian       = ">"       # Standard size
    
class Element(object):
    """A single element in a struct."""
    id=0
    def __init__(self, typecode):
        Element.id+=1           # Note: not thread safe
        self.id = Element.id
        self.typecode = typecode
        self.size = struct.calcsize(typecode)

    def __len__(self):
        return self.size

    def decode(self, format, s):
        """Additional decode steps once converted via struct.unpack"""
        return s

    def encode(self, format, val):
        """Additional encode steps to allow packing with struct.pack"""
        return val

    def __str__(self):
        return self.typecode

    def __call__(self, num):
        """Define this as an array of elements."""
        # Special case - strings already handled as one blob.
        if self.typecode in 'sp':
            # Strings handled specially - only one item
            return Element('%ds' % num)
        else:
            return ArrayElement(self, num)

    def __getitem__(self, num): return self(num)

class ArrayElement(Element):
    def __init__(self, basic_element, num):
        Element.__init__(self, '%ds' % (len(basic_element) * num))
        self.num = num
        self.basic_element = basic_element

    def decode(self, format, s):
        # NB. We use typecode * size, not %s%s' % (size, typecode), 
        # so we deal with typecodes that already have numbers,  
        # ie 2*'4s' != '24s'
        return [self.basic_element.decode(format, x) for x in  
                    struct.unpack('%s%s' % (format, 
                            self.num * self.basic_element.typecode),s)]

    def encode(self, format, vals):
        fmt = format + (self.basic_element.typecode * self.num)
        return struct.pack(fmt, *[self.basic_element.encode(format,v) 
                                  for v in vals])

class EmbeddedStructElement(Element):
    def __init__(self, structure):
        Element.__init__(self, '%ds' % structure._struct_size)
        self.struct = structure

    # Note: Structs use their own endianness format, not their parent's
    def decode(self, format, s):
        return self.struct(s)

    def encode(self, format, s):
        return self.struct._pack(s)

name_to_code = {
    'Char'             : 'c',
    'Byte'             : 'b',
    'UnsignedByte'     : 'B',
    'Int'              : 'i',
    'UnsignedInt'      : 'I',
    'Short'            : 'h',
    'UnsignedShort'    : 'H',
    'Long'             : 'l',
    'UnsignedLong'     : 'L',
    'String'           : 's',  
    'PascalString'     : 'p',  
    'Pointer'          : 'P',
    'Float'            : 'f',
    'Double'           : 'd',
    'LongLong'         : 'q',
    'UnsignedLongLong' : 'Q',
    }


class _NullPaddedString(Element):
    def __init__(self, count):
        super(_NullPaddedString, self).__init__('%ds' % count)

    def decode(self, format, s):
        return s.rstrip(b"\x00")

class _Type(object):
    def __getattr__(self, name):
        return Element(name_to_code[name])

    def Struct(self, struct):
        return EmbeddedStructElement(struct)

    NullPaddedString = _NullPaddedString

Type=_Type()

class MetaStruct(type):
    def __init__(cls, name, bases, d):
        type.__init__(cls, name, bases, d)
        if hasattr(cls, '_struct_data'):  # Allow extending by inheritance
            cls._struct_info = list(cls._struct_info) # use copy.
        else:
            cls._struct_data = ''
            cls._struct_info = []     # name / element pairs

        # Get each Element field, sorted by id.
        elems = sorted(((k, v) for k, v in six.iteritems(d) if isinstance(v, Element)), key=lambda x: x[1].id)

        cls._struct_data += ''.join(str(v) for _, v in elems)
        cls._struct_info += elems
        cls._struct_size = struct.calcsize(cls._format + cls._struct_data)

    def __len__(cls):
        return cls._struct_size

@six.add_metaclass(MetaStruct)
class Struct(object):
    """Represent a binary structure."""
    _format = Format.Native  # Default to native format, native size

    def __init__(self, _data=None, **kwargs):
        if _data is None:
            _data = b'\0' * self._struct_size
            
        fieldvals = zip(self._struct_info, struct.unpack(self._format + self._struct_data, _data))
        for (name, elem), val in fieldvals:
            setattr(self, name, elem.decode(self._format, val))
        
        for k,v in six.iteritems(kwargs):
            setattr(self, k, v)

    def _pack(self):
        try:
            args = [elem.encode(self._format, getattr(self, name)) for (name, elem) in self._struct_info]
            return struct.pack(self._format + self._struct_data, *args)
        except struct.error:
            for fmt, data in zip(self._struct_info, args):
                print(fmt[0], fmt[1], data)
                struct.pack(str(fmt[1]), data)
            exit(0)

    def __str__(self):
        if six.PY3:
            raise ValueError("Use bytes() on this to get the binary representation of this structure.")
        return self._pack()

    def __bytes__(self):
        return self._pack()

    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self._pack())

    @classmethod
    def offsetof(cls, name):
        offset = 0
        for (tag, elem) in cls._struct_info:
            if name == tag:
                return offset
            offset += elem.size

        raise KeyError()

    def member_info(self, name):
        for (tag, elem) in self._struct_info:
            if name == tag:
                return elem

        raise KeyError()