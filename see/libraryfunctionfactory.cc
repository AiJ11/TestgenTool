#include "libraryfunctionfactory.hh"
#include <iostream>
#include <stdexcept>

using namespace std;

namespace Library
{

    /* ============================================================
     * LibraryAPIFunction Base Implementation
     * ============================================================ */

    LibraryAPIFunction::LibraryAPIFunction(LibraryFunctionFactory *factory, vector<Expr *> args)
        : factory(factory), arguments(args) {}

    string LibraryAPIFunction::extractString(Expr *expr)
    {
        if (!expr)
            throw runtime_error("Null expression in extractString");

        if (expr->exprType == ExprType::STRING)
        {
            String *str = dynamic_cast<String *>(expr);
            return str->value;
        }

        if (expr->exprType == ExprType::VAR)
        {
            Var *var = dynamic_cast<Var *>(expr);
            return var->name;
        }

        if (expr->exprType == ExprType::NUM)
        {
            Num *num = dynamic_cast<Num *>(expr);
            return to_string(num->value);
        }

        throw runtime_error("Expected STRING, VAR, or NUM expression");
    }

    int LibraryAPIFunction::extractInt(Expr *expr)
    {
        if (!expr)
            throw runtime_error("Null expression in extractInt");

        if (expr->exprType == ExprType::NUM)
        {
            Num *num = dynamic_cast<Num *>(expr);
            return num->value;
        }

        if (expr->exprType == ExprType::STRING)
        {
            String *str = dynamic_cast<String *>(expr);
            return stoi(str->value);
        }

        throw runtime_error("Expected NUM expression");
    }

    json LibraryAPIFunction::extractJson(Expr *expr)
    {
        if (!expr)
            return json::object();

        if (expr->exprType == ExprType::MAP)
        {
            Map *map = dynamic_cast<Map *>(expr);
            json obj = json::object();

            for (const auto &kv : map->value)
            {
                string key = kv.first->name;

                if (kv.second->exprType == ExprType::STRING)
                {
                    String *val = dynamic_cast<String *>(kv.second.get());
                    obj[key] = val->value;
                }
                else if (kv.second->exprType == ExprType::NUM)
                {
                    Num *val = dynamic_cast<Num *>(kv.second.get());
                    obj[key] = val->value;
                }
                else
                {
                    obj[key] = "unsupported_type";
                }
            }

            return obj;
        }

        if (expr->exprType == ExprType::STRING)
        {
            String *str = dynamic_cast<String *>(expr);
            try
            {
                return json::parse(str->value);
            }
            catch (...)
            {
                return str->value;
            }
        }

        return json::object();
    }

    /* ============================================================
     * Test API Functions
     * ============================================================ */

    ResetFunc::ResetFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> ResetFunc::execute()
    {
        cout << "[ResetFunc] Clearing all collections..." << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->del("/api/test/clear_all");

            if (resp.statusCode >= 200 && resp.statusCode < 300)
            {
                factory->getB().clear();
                factory->getS().clear();
                factory->getReq().clear();
                factory->getLoans().clear();
                return make_unique<Num>(200);
            }
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[ResetFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    // get_B (Books)
    GetBFunc::GetBFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetBFunc::execute()
    {
        cout << "[GetBFunc] Fetching B (Books)..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/api/test/get_B");

            vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getB().clear();

                for (auto &[k, v] : data.items())
                {
                    string key = k;
                    string value = v.dump();
                    factory->getB()[key] = value;

                    pairs.push_back(make_pair(
                        make_unique<Var>(key),
                        make_unique<String>(value)));
                }
            }

            return make_unique<Map>(std::move(pairs));
        }
        catch (const exception &e)
        {
            cerr << "[GetBFunc] Error: " << e.what() << endl;
            return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
        }
    }

    SetBFunc::SetBFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SetBFunc::execute()
    {
        cout << "[SetBFunc] Setting B (Books)..." << endl;
        // Not typically needed for library system
        return make_unique<Num>(200);
    }

    // get_S (Students)
    GetSFunc::GetSFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetSFunc::execute()
    {
        cout << "[GetSFunc] Fetching S (Students)..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/api/test/get_S");

            vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getS().clear();

                for (auto &[k, v] : data.items())
                {
                    string key = k;
                    string value = v.dump();
                    factory->getS()[key] = value;

                    pairs.push_back(make_pair(
                        make_unique<Var>(key),
                        make_unique<String>(value)));
                }
            }

            return make_unique<Map>(std::move(pairs));
        }
        catch (const exception &e)
        {
            cerr << "[GetSFunc] Error: " << e.what() << endl;
            return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
        }
    }

    SetSFunc::SetSFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SetSFunc::execute()
    {
        cout << "[SetSFunc] Setting S (Students)..." << endl;
        return make_unique<Num>(200);
    }

    // get_Req (Requests)
    GetReqFunc::GetReqFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetReqFunc::execute()
    {
        cout << "[GetReqFunc] Fetching Req (Requests)..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Req");

            vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getReq().clear();

                for (auto &[k, v] : data.items())
                {
                    string key = k;
                    string value = v.dump();
                    factory->getReq()[key] = value;

                    pairs.push_back(make_pair(
                        make_unique<Var>(key),
                        make_unique<String>(value)));
                }
            }

            return make_unique<Map>(std::move(pairs));
        }
        catch (const exception &e)
        {
            cerr << "[GetReqFunc] Error: " << e.what() << endl;
            return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
        }
    }

    SetReqFunc::SetReqFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SetReqFunc::execute()
    {
        cout << "[SetReqFunc] Setting Req (Requests)..." << endl;
        return make_unique<Num>(200);
    }

    // get_Loans
    GetLoansFunc::GetLoansFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetLoansFunc::execute()
    {
        cout << "[GetLoansFunc] Fetching Loans..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/api/test/get_Loans");

            vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;

            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getLoans().clear();

                for (auto &[k, v] : data.items())
                {
                    string key = k;
                    string value = v.dump();
                    factory->getLoans()[key] = value;

                    pairs.push_back(make_pair(
                        make_unique<Var>(key),
                        make_unique<String>(value)));
                }
            }

            return make_unique<Map>(std::move(pairs));
        }
        catch (const exception &e)
        {
            cerr << "[GetLoansFunc] Error: " << e.what() << endl;
            return make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{});
        }
    }

    SetLoansFunc::SetLoansFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SetLoansFunc::execute()
    {
        cout << "[SetLoansFunc] Setting Loans..." << endl;
        return make_unique<Num>(200);
    }

    /* ============================================================
     * Book API Functions
     * ============================================================ */

    GetAllBooksFunc::GetAllBooksFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetAllBooksFunc::execute()
    {
        cout << "[GetAllBooksFunc] Getting all books..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/books/getAll");
            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getB().clear();
                for (const auto &book : data)
                {
                    string key = to_string(book["bookCode"].get<int>());
                    factory->getB()[key] = book.dump();
                }
            }
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetAllBooksFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    GetBookByCodeFunc::GetBookByCodeFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetBookByCodeFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("getBookByCode requires 1 argument");

        string bookCode = extractString(arguments[0]);
        cout << "[GetBookByCodeFunc] Getting book: " << bookCode << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/books/getBook/" + bookCode);
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetBookByCodeFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    SaveBookFunc::SaveBookFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SaveBookFunc::execute()
    {
        if (arguments.size() < 3)
            throw runtime_error("saveBook requires 3 arguments");

        string bookTitle = extractString(arguments[0]);
        string bookAuthor = extractString(arguments[1]);
        string bookDesc = extractString(arguments[2]);

        cout << "[SaveBookFunc] Creating book: " << bookTitle << endl;

        try
        {
            json body = {
                {"bookTitle", bookTitle},
                {"author", bookAuthor},
                {"bookDesc", bookDesc}};

            HttpResponse resp = factory->getHttpClient()->post("/books/save", body);

            if (resp.statusCode == 200 || resp.statusCode == 201)
            {
                json data = resp.getJson();
                string bookCode = to_string(data["bookCode"].get<int>());
                factory->getB()[bookCode] = data.dump();
                cout << "[SaveBookFunc] Created book with code: " << bookCode << endl;
                return make_unique<String>(bookCode);
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[SaveBookFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    UpdateBookFunc::UpdateBookFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> UpdateBookFunc::execute()
    {
        if (arguments.size() < 4)
            throw runtime_error("updateBook requires 4 arguments");

        string bookCode = extractString(arguments[0]);
        string bookTitle = extractString(arguments[1]);
        string bookAuthor = extractString(arguments[2]);
        string bookDesc = extractString(arguments[3]);

        cout << "[UpdateBookFunc] Updating book: " << bookCode << endl;

        try
        {
            json body = {
                {"bookCode", stoi(bookCode)},
                {"bookTitle", bookTitle},
                {"author", bookAuthor},
                {"bookDesc", bookDesc}};

            HttpResponse resp = factory->getHttpClient()->put("/books/updateBook", body);

            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getB()[bookCode] = data.dump();
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[UpdateBookFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    DeleteBookFunc::DeleteBookFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> DeleteBookFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("deleteBook requires 1 argument");

        string bookCode = extractString(arguments[0]);
        cout << "[DeleteBookFunc] Deleting book: " << bookCode << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->del("/books/deleteBook/" + bookCode);

            if (resp.statusCode == 200)
            {
                factory->getB().erase(bookCode);
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[DeleteBookFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    /* ============================================================
     * Student API Functions
     * ============================================================ */

    GetAllStudentsFunc::GetAllStudentsFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetAllStudentsFunc::execute()
    {
        cout << "[GetAllStudentsFunc] Getting all students..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/student/getAll");
            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getS().clear();
                for (const auto &student : data)
                {
                    string key = to_string(student["id"].get<int>());
                    factory->getS()[key] = student.dump();
                }
            }
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetAllStudentsFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    GetStudentByIdFunc::GetStudentByIdFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetStudentByIdFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("getStudentById requires 1 argument");

        string studentId = extractString(arguments[0]);
        cout << "[GetStudentByIdFunc] Getting student: " << studentId << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/student/getStudent/" + studentId);
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetStudentByIdFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    SaveStudentFunc::SaveStudentFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SaveStudentFunc::execute()
    {
        if (arguments.size() < 3)
            throw runtime_error("saveStudent requires 3 arguments");

        string studentName = extractString(arguments[0]);
        string studentEmail = extractString(arguments[1]);
        string studentPhone = extractString(arguments[2]);

        cout << "[SaveStudentFunc] Creating student: " << studentName << endl;

        try
        {
            json body = {
                {"studentName", studentName},
                {"email", studentEmail},
                {"phone", studentPhone}};

            // Using userId=1 (default test user)
            HttpResponse resp = factory->getHttpClient()->post("/student/save?userId=1", body);

            if (resp.statusCode == 200 || resp.statusCode == 201)
            {
                json data = resp.getJson();
                string studentId = to_string(data["id"].get<int>());
                factory->getS()[studentId] = data.dump();
                cout << "[SaveStudentFunc] Created student with id: " << studentId << endl;
                return make_unique<String>(studentId);
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[SaveStudentFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    UpdateStudentFunc::UpdateStudentFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> UpdateStudentFunc::execute()
    {
        if (arguments.size() < 4)
            throw runtime_error("updateStudent requires 4 arguments");

        string studentId = extractString(arguments[0]);
        string studentName = extractString(arguments[1]);
        string studentEmail = extractString(arguments[2]);
        string studentPhone = extractString(arguments[3]);

        cout << "[UpdateStudentFunc] Updating student: " << studentId << endl;

        try
        {
            json body = {
                {"id", stoi(studentId)},
                {"studentName", studentName},
                {"email", studentEmail},
                {"phone", studentPhone}};

            HttpResponse resp = factory->getHttpClient()->put("/student/updateStudent?userId=1", body);

            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getS()[studentId] = data.dump();
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[UpdateStudentFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    DeleteStudentFunc::DeleteStudentFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> DeleteStudentFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("deleteStudent requires 1 argument");

        string studentId = extractString(arguments[0]);
        cout << "[DeleteStudentFunc] Deleting student: " << studentId << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->del("/student/deleteStudent/" + studentId);

            if (resp.statusCode == 200)
            {
                factory->getS().erase(studentId);
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[DeleteStudentFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    /* ============================================================
     * Request API Functions
     * ============================================================ */

    GetAllRequestsFunc::GetAllRequestsFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetAllRequestsFunc::execute()
    {
        cout << "[GetAllRequestsFunc] Getting all requests..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/requests/allRequests");
            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getReq().clear();
                for (const auto &req : data)
                {
                    string key = to_string(req["slno"].get<int>());
                    factory->getReq()[key] = req.dump();
                }
            }
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetAllRequestsFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    GetRequestByIdFunc::GetRequestByIdFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetRequestByIdFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("getRequestById requires 1 argument");

        string requestId = extractString(arguments[0]);
        cout << "[GetRequestByIdFunc] Getting request: " << requestId << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/requests/" + requestId);
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetRequestByIdFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    SaveRequestFunc::SaveRequestFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SaveRequestFunc::execute()
    {
        if (arguments.size() < 4)
            throw runtime_error("saveRequest requires 4 arguments");

        string studentId = extractString(arguments[0]);
        string bookCode = extractString(arguments[1]);
        string startDate = extractString(arguments[2]);
        string endDate = extractString(arguments[3]);

        cout << "[SaveRequestFunc] Creating request: student=" << studentId << " book=" << bookCode << endl;

        try
        {
            json body = {
                {"student", {{"id", stoi(studentId)}}},
                {"book", {{"bookCode", stoi(bookCode)}}},
                {"startDate", startDate},
                {"endDate", endDate}};

            HttpResponse resp = factory->getHttpClient()->post("/requests/save?userId=1", body);

            if (resp.statusCode == 200 || resp.statusCode == 201)
            {
                json data = resp.getJson();
                string slno = to_string(data["slno"].get<int>());
                factory->getReq()[slno] = data.dump();
                cout << "[SaveRequestFunc] Created request with slno: " << slno << endl;
                return make_unique<String>(slno);
            }

            cerr << "[SaveRequestFunc] Error response: " << resp.body << endl;
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[SaveRequestFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    DeleteRequestFunc::DeleteRequestFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> DeleteRequestFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("deleteRequest requires 1 argument");

        string requestId = extractString(arguments[0]);
        cout << "[DeleteRequestFunc] Deleting request: " << requestId << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->del("/requests/delete/" + requestId);

            if (resp.statusCode == 200)
            {
                factory->getReq().erase(requestId);
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[DeleteRequestFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    /* ============================================================
     * Loan (BookStudent) API Functions
     * ============================================================ */

    GetAllLoansFunc::GetAllLoansFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetAllLoansFunc::execute()
    {
        cout << "[GetAllLoansFunc] Getting all loans..." << endl;
        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/bookStudent/getAll");
            if (resp.statusCode == 200)
            {
                json data = resp.getJson();
                factory->getLoans().clear();
                for (const auto &loan : data)
                {
                    string key = to_string(loan["slno"].get<int>());
                    factory->getLoans()[key] = loan.dump();
                }
            }
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetAllLoansFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    GetLoanByIdFunc::GetLoanByIdFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> GetLoanByIdFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("getLoanById requires 1 argument");

        string loanId = extractString(arguments[0]);
        cout << "[GetLoanByIdFunc] Getting loan: " << loanId << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->get("/bookStudent/" + loanId);
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[GetLoanByIdFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    AcceptRequestFunc::AcceptRequestFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> AcceptRequestFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("acceptRequest requires 1 argument");

        string requestId = extractString(arguments[0]);
        cout << "[AcceptRequestFunc] Accepting request: " << requestId << endl;

        try
        {
            // First, fetch the request to get its full data
            HttpResponse getResp = factory->getHttpClient()->get("/requests/" + requestId);
            if (getResp.statusCode != 200)
            {
                cerr << "[AcceptRequestFunc] Request not found: " << requestId << endl;
                return make_unique<Num>(404);
            }

            json requestData = getResp.getJson();

            // Accept the request
            HttpResponse resp = factory->getHttpClient()->post("/bookStudent/accept?userId=1", requestData);

            if (resp.statusCode == 200 || resp.statusCode == 201)
            {
                json data = resp.getJson();
                string loanSlno = to_string(data["slno"].get<int>());

                // Update caches: remove request, add loan
                factory->getReq().erase(requestId);
                factory->getLoans()[loanSlno] = data.dump();

                cout << "[AcceptRequestFunc] Created loan " << loanSlno << " from request " << requestId << endl;
                return make_unique<String>(loanSlno);
            }

            cerr << "[AcceptRequestFunc] Error: " << resp.body << endl;
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[AcceptRequestFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    ReturnBookFunc::ReturnBookFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> ReturnBookFunc::execute()
    {
        if (arguments.size() < 1)
            throw runtime_error("returnBook requires 1 argument");

        string loanId = extractString(arguments[0]);
        cout << "[ReturnBookFunc] Returning book (loan): " << loanId << endl;

        try
        {
            HttpResponse resp = factory->getHttpClient()->del("/bookStudent/delete/" + loanId);

            if (resp.statusCode == 200 || resp.statusCode == 204)
            {
                factory->getLoans().erase(loanId);
                cout << "[ReturnBookFunc] Book returned successfully" << endl;
            }

            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[ReturnBookFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    SaveLoanFunc::SaveLoanFunc(LibraryFunctionFactory *factory, vector<Expr *> args)
        : LibraryAPIFunction(factory, args) {}

    unique_ptr<Expr> SaveLoanFunc::execute()
    {
        if (arguments.size() < 4)
            throw runtime_error("saveLoan requires 4 arguments");

        string studentId = extractString(arguments[0]);
        string bookCode = extractString(arguments[1]);
        string startDate = extractString(arguments[2]);
        string endDate = extractString(arguments[3]);

        cout << "[SaveLoanFunc] Creating direct loan: student=" << studentId << " book=" << bookCode << endl;

        try
        {
            json body = {
                {"student", {{"id", stoi(studentId)}}},
                {"book", {{"bookCode", stoi(bookCode)}}},
                {"startDate", startDate},
                {"endDate", endDate}};

            HttpResponse resp = factory->getHttpClient()->post("/bookStudent/save", body);

            if (resp.statusCode == 200 || resp.statusCode == 201)
            {
                json data = resp.getJson();
                string slno = to_string(data["slno"].get<int>());
                factory->getLoans()[slno] = data.dump();
                cout << "[SaveLoanFunc] Created loan with slno: " << slno << endl;
                return make_unique<String>(slno);
            }

            cerr << "[SaveLoanFunc] Error: " << resp.body << endl;
            return make_unique<Num>(resp.statusCode);
        }
        catch (const exception &e)
        {
            cerr << "[SaveLoanFunc] Error: " << e.what() << endl;
            return make_unique<Num>(500);
        }
    }

    /* ============================================================
     * LibraryFunctionFactory Implementation
     * ============================================================ */

    LibraryFunctionFactory::LibraryFunctionFactory(const string &baseUrl)
        : baseUrl(baseUrl)
    {
        httpClient = make_unique<HttpClient>(baseUrl);
        cout << "[LibraryFunctionFactory] Initialized with baseUrl: " << baseUrl << endl;
    }

    unique_ptr<Function> LibraryFunctionFactory::getFunction(string fname, vector<Expr *> args)
    {
        cout << "[LibraryFactory] Creating function: " << fname << endl;

        // Test API functions
        if (fname == "reset")
            return make_unique<ResetFunc>(this, args);
        if (fname == "get_B")
            return make_unique<GetBFunc>(this, args);
        if (fname == "set_B")
            return make_unique<SetBFunc>(this, args);
        if (fname == "get_S")
            return make_unique<GetSFunc>(this, args);
        if (fname == "set_S")
            return make_unique<SetSFunc>(this, args);
        if (fname == "get_Req")
            return make_unique<GetReqFunc>(this, args);
        if (fname == "set_Req")
            return make_unique<SetReqFunc>(this, args);
        if (fname == "get_Loans")
            return make_unique<GetLoansFunc>(this, args);
        if (fname == "set_Loans")
            return make_unique<SetLoansFunc>(this, args);

        // Book API functions
        if (fname == "getAllBooks")
            return make_unique<GetAllBooksFunc>(this, args);
        if (fname == "getBookByCode")
            return make_unique<GetBookByCodeFunc>(this, args);
        if (fname == "saveBook")
            return make_unique<SaveBookFunc>(this, args);
        if (fname == "updateBook")
            return make_unique<UpdateBookFunc>(this, args);
        if (fname == "deleteBook")
            return make_unique<DeleteBookFunc>(this, args);

        // Student API functions
        if (fname == "getAllStudents")
            return make_unique<GetAllStudentsFunc>(this, args);
        if (fname == "getStudentById")
            return make_unique<GetStudentByIdFunc>(this, args);
        if (fname == "saveStudent")
            return make_unique<SaveStudentFunc>(this, args);
        if (fname == "updateStudent")
            return make_unique<UpdateStudentFunc>(this, args);
        if (fname == "deleteStudent")
            return make_unique<DeleteStudentFunc>(this, args);

        // Request API functions
        if (fname == "getAllRequests")
            return make_unique<GetAllRequestsFunc>(this, args);
        if (fname == "getRequestById")
            return make_unique<GetRequestByIdFunc>(this, args);
        if (fname == "saveRequest")
            return make_unique<SaveRequestFunc>(this, args);
        if (fname == "deleteRequest")
            return make_unique<DeleteRequestFunc>(this, args);

        // Loan API functions
        if (fname == "getAllLoans")
            return make_unique<GetAllLoansFunc>(this, args);
        if (fname == "getLoanById")
            return make_unique<GetLoanByIdFunc>(this, args);
        if (fname == "acceptRequest")
            return make_unique<AcceptRequestFunc>(this, args);
        if (fname == "returnBook")
            return make_unique<ReturnBookFunc>(this, args);
        if (fname == "saveLoan")
            return make_unique<SaveLoanFunc>(this, args);

        throw runtime_error("Unknown function: " + fname);
    }

} // namespace Library
