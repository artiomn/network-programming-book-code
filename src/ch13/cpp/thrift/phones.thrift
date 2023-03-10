enum PhoneType {
    HOME,
    WORK,
    MOBILE,
    OTHER
}


struct Phone {
    1: i32 id,
    2: string number,
    3: PhoneType type
}


service PhoneService {
    Phone findById(1: i32 id),
    list<Phone> findAll()
}
