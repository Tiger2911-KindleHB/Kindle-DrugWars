#include <gtk/gtk.h>
#include <pango/pango.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct ItemDef {
    const char* name;
    int low;
    int high;
};

static const ItemDef ITEM_DEFS[] = {
    {"Acid", 1000, 4400},
    {"Cocaine", 15000, 29000},
    {"Hashish", 480, 1280},
    {"Heroin", 5500, 13000},
    {"Ludes", 11, 60},
    {"MDA", 1500, 4400},
    {"Opium", 540, 1250},
    {"PCP", 1000, 2500},
    {"Peyote", 220, 700},
    {"Shrooms", 630, 1300},
    {"Speed", 90, 250},
    {"Weed", 315, 890},
};

static const char* LOCATIONS[] = {
    "Bronx",
    "Ghetto",
    "Central Park",
    "Manhattan",
    "Coney Island",
    "Brooklyn",
};

constexpr int ITEM_COUNT = sizeof(ITEM_DEFS) / sizeof(ITEM_DEFS[0]);
constexpr int LOCATION_COUNT = sizeof(LOCATIONS) / sizeof(LOCATIONS[0]);
constexpr int START_CASH = 2000;
constexpr int START_DEBT = 5500;
constexpr int START_CAPACITY = 100;
constexpr int START_HEALTH = 100;
constexpr int GUN_PRICE = 400;
constexpr int COAT_PRICE = 2000;
constexpr int COAT_SIZE = 100;
constexpr int LOAN_INCREMENT = 1000;
constexpr int NEGATIVE_LIMIT = 7;
constexpr int STREET_TIP_BASE_COST = 250;


std::string money(long long value) {
    bool neg = value < 0;
    unsigned long long n = neg ? static_cast<unsigned long long>(-value) : static_cast<unsigned long long>(value);
    std::string s = std::to_string(n);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) {
        s.insert(static_cast<size_t>(i), ",");
    }
    return std::string(neg ? "-$" : "$") + s;
}

int clampi(int v, int lo, int hi) {
    return std::max(lo, std::min(hi, v));
}

long long parse_ll(const std::string& s, long long fallback = 0) {
    char* end = nullptr;
    errno = 0;
    long long v = std::strtoll(s.c_str(), &end, 10);
    if (errno != 0 || end == s.c_str()) return fallback;
    return v;
}

int parse_int(const std::string& s, int fallback = 0) {
    return static_cast<int>(parse_ll(s, fallback));
}

std::string save_path() {
    const char* env = std::getenv("KINDLEDOPEWARS_SAVE");
    if (env && *env) return std::string(env);
    const char* home = std::getenv("HOME");
    if (home && *home) return std::string(home) + "/.kindledopewars.save";
    return "/tmp/kindledopewars.save";
}

std::string data_dir_path() {
    std::string path = save_path();
    size_t slash = path.find_last_of('/');
    if (slash == std::string::npos || slash == 0) return "/tmp";
    return path.substr(0, slash);
}

std::string ui_debug_path() {
    return data_dir_path() + "/ui-debug.txt";
}

class GameApp {
public:
    GtkWidget* window = nullptr;
    GtkWidget* root = nullptr;

    int day = 1;
    long long cash = START_CASH;
    long long debt = START_DEBT;
    long long bank = 0;
    int health = START_HEALTH;
    int capacity = START_CAPACITY;
    int guns = 0;
    int location = 0;
    int selected = 0;
    int negative_days = 0;
    int future_event_day = 0;
    int future_event_item = 0;
    int future_event_type = 0; // 1 shortage/spike, 2 flood/crash
    int future_event_location = 0;
    bool future_event_known = false;
    int cops = 0;
    int offer_type = 0; // 1 gun, 2 coat
    bool game_over = false;
    std::string game_over_reason;
    std::string log = "Welcome to New York. Buy low, sell high, pay the loan shark.";
    int header_font = 9;
    int info_font = 11;
    int event_font = 14;
    int chart_font = 6;
    int bottom_font = 9;
    int section_gap = 0;
    int chart_gap = -8;
    int bottom_gap = 0;
    int header_height_delta = 0;
    int stats_height_delta = 0;
    int event_height_delta = 0;
    int chart_height_delta = -80;
    int chart_row_delta = -10;
    int bottom_height_delta = 0;
    std::vector<int> price;
    std::vector<int> held;
    std::mt19937 rng;

    GameApp() : price(ITEM_COUNT, 0), held(ITEM_COUNT, 0) {
        rng.seed(static_cast<unsigned int>(std::time(nullptr)) ^ 0xBADC0DEu);
    }

    int rand_int(int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    }

    int used_space() const {
        int used = 0;
        for (int q : held) used += q;
        return used;
    }

    long long inventory_value() const {
        long long total = 0;
        for (int i = 0; i < ITEM_COUNT; ++i) total += static_cast<long long>(held[i]) * price[i];
        return total;
    }

    long long net_worth() const {
        return cash + bank + inventory_value() - debt;
    }

    void new_game() {
        day = 1;
        cash = START_CASH;
        debt = START_DEBT;
        bank = 0;
        health = START_HEALTH;
        capacity = START_CAPACITY;
        guns = 0;
        location = 0;
        selected = 0;
        negative_days = 0;
        future_event_day = 0;
        future_event_item = 0;
        future_event_type = 0;
        future_event_location = 0;
        future_event_known = false;
        cops = 0;
        offer_type = 0;
        game_over = false;
        game_over_reason.clear();
        std::fill(held.begin(), held.end(), 0);
        generate_market();
        schedule_future_event();
        log = "New run started. There is no day limit. Negative cash for 8 straight days ends the run.";
        save();
    }

    void generate_market() {
        for (int i = 0; i < ITEM_COUNT; ++i) {
            price[i] = rand_int(ITEM_DEFS[i].low, ITEM_DEFS[i].high);
        }

        int event_roll = rand_int(1, 100);
        if (event_roll <= 12) {
            int idx = rand_int(0, ITEM_COUNT - 1);
            price[idx] *= rand_int(3, 7);
            std::ostringstream ss;
            ss << "The market is dry. " << ITEM_DEFS[idx].name << " prices are outrageous.";
            log = ss.str();
        } else if (event_roll <= 24) {
            int idx = rand_int(0, ITEM_COUNT - 1);
            price[idx] = std::max(1, price[idx] / rand_int(3, 6));
            std::ostringstream ss;
            ss << "A huge supply hit the street. " << ITEM_DEFS[idx].name << " is dirt cheap.";
            log = ss.str();
        }
    }

    void schedule_future_event() {
        future_event_day = day + rand_int(2, 5);
        future_event_item = rand_int(0, ITEM_COUNT - 1);
        future_event_type = rand_int(1, 2);
        future_event_location = rand_int(0, LOCATION_COUNT - 1);
        future_event_known = false;
    }

    int tip_cost() const {
        return STREET_TIP_BASE_COST + std::max(0, day - 1) * 50;
    }

    std::string future_tip_text() const {
        int days_left = std::max(0, future_event_day - day);
        std::ostringstream ss;
        ss << "Street tip: ";
        if (future_event_type == 1) {
            ss << ITEM_DEFS[future_event_item].name << " may get scarce in ";
        } else {
            ss << ITEM_DEFS[future_event_item].name << " may flood the street in ";
        }
        ss << LOCATIONS[future_event_location] << " in ";
        ss << days_left << " day" << (days_left == 1 ? "" : "s") << ".";
        return ss.str();
    }

    void buy_street_tip() {
        if (game_over) return;
        if (future_event_day <= day || future_event_type < 1 || future_event_type > 2) {
            schedule_future_event();
        }
        int cost = tip_cost();
        if (cash < cost) {
            log = "Street information costs " + money(cost) + ". You do not have enough cash.";
        } else {
            cash -= cost;
            future_event_known = true;
            log = future_tip_text();
        }
        save();
        show_market();
    }

    void apply_future_event_if_due() {
        if (future_event_day <= 0 || future_event_type < 1 || future_event_type > 2) {
            schedule_future_event();
        }
        if (day < future_event_day) return;

        if (location == future_event_location) {
            if (future_event_type == 1) {
                price[future_event_item] *= rand_int(4, 8);
                log = std::string("Rumors were right. ") + ITEM_DEFS[future_event_item].name + " is scarce here. Prices spiked.";
            } else {
                price[future_event_item] = std::max(1, price[future_event_item] / rand_int(4, 8));
                log = std::string("Rumors were right. ") + ITEM_DEFS[future_event_item].name + " is everywhere here. Prices crashed.";
            }
        }
        schedule_future_event();
    }

    void daily_interest() {
        debt += std::max<long long>(1, debt / 10); // classic loan-shark pressure: 10% daily
        if (bank > 0) bank += std::max<long long>(1, bank / 100); // bank grows slowly; debt stays dangerous
    }

    void check_negative_streak() {
        if (cash < 0) {
            negative_days += 1;
        } else {
            negative_days = 0;
        }
        if (negative_days > NEGATIVE_LIMIT) {
            end_game("You stayed negative for more than seven days in a row.");
        }
    }

    void end_game(const std::string& reason) {
        game_over = true;
        game_over_reason = reason;
        save();
    }

    void maybe_random_event() {
        int roll = rand_int(1, 100);
        if (roll <= 9) {
            cops = rand_int(2, 7) + day / 25;
            log = "Officer Hardass spotted you.";
            save();
            show_police();
            return;
        }
        if (roll <= 15) {
            long long loss = std::min<long long>(std::max<long long>(100, cash / 5), rand_int(200, 2200));
            cash -= loss;
            log = "You were mugged. Lost " + money(loss) + ".";
            return;
        }
        if (roll <= 21) {
            long long found = rand_int(100, 1500);
            cash += found;
            log = "You found " + money(found) + " in a trash can.";
            return;
        }
        if (roll <= 27 && used_space() < capacity) {
            int idx = rand_int(0, ITEM_COUNT - 1);
            int qty = std::min(capacity - used_space(), rand_int(1, 8));
            held[idx] += qty;
            std::ostringstream ss;
            ss << "A stranger dropped " << qty << " " << ITEM_DEFS[idx].name << ".";
            log = ss.str();
            return;
        }
        if (roll <= 31 && cash >= GUN_PRICE) {
            offer_type = 1;
            log = "A gun dealer offers a .38 for " + money(GUN_PRICE) + ".";
            save();
            show_offer();
            return;
        }
        if (roll <= 34 && cash >= COAT_PRICE) {
            offer_type = 2;
            log = "A street vendor offers a bigger trenchcoat for " + money(COAT_PRICE) + ".";
            save();
            show_offer();
            return;
        }
        if (roll <= 39 && used_space() > 0) {
            std::vector<int> nonempty;
            for (int i = 0; i < ITEM_COUNT; ++i) if (held[i] > 0) nonempty.push_back(i);
            if (!nonempty.empty()) {
                int idx = nonempty[static_cast<size_t>(rand_int(0, static_cast<int>(nonempty.size()) - 1))];
                int qty = std::min(held[idx], rand_int(1, 5));
                held[idx] -= qty;
                std::ostringstream ss;
                ss << "A bad deal went sideways. Lost " << qty << " " << ITEM_DEFS[idx].name << ".";
                log = ss.str();
            }
            return;
        }
        log = "The subway ride was quiet.";
    }

    void travel_to(int dest) {
        if (dest == location) {
            log = "You are already in " + std::string(LOCATIONS[location]) + ".";
            save();
            show_market();
            return;
        }
        location = dest;
        day += 1;
        daily_interest();
        generate_market();
        apply_future_event_if_due();
        maybe_random_event();
        if (!game_over) check_negative_streak();
        save();
        if (!game_over && cops == 0 && offer_type == 0) show_market();
        if (game_over) show_game_over();
    }

    void buy_selected(int requested) {
        if (selected < 0 || selected >= ITEM_COUNT) return;
        int space = capacity - used_space();
        if (space <= 0) {
            log = "Your trenchcoat is full.";
            show_market();
            return;
        }
        if (price[selected] <= 0 || cash <= 0) {
            log = "You do not have enough cash to buy.";
            show_market();
            return;
        }
        int affordable = static_cast<int>(std::min<long long>(space, cash / price[selected]));
        int qty = std::min(requested, affordable);
        if (requested >= 9999) qty = affordable;
        if (qty <= 0) {
            log = "You do not have enough cash to buy.";
        } else {
            held[selected] += qty;
            cash -= static_cast<long long>(qty) * price[selected];
            std::ostringstream ss;
            ss << "Bought " << qty << " " << ITEM_DEFS[selected].name << ".";
            log = ss.str();
        }
        save();
        show_market();
    }

    void sell_selected(int requested) {
        if (selected < 0 || selected >= ITEM_COUNT) return;
        int qty = std::min(requested, held[selected]);
        if (requested >= 9999) qty = held[selected];
        if (qty <= 0) {
            log = "You do not have any to sell.";
        } else {
            held[selected] -= qty;
            cash += static_cast<long long>(qty) * price[selected];
            std::ostringstream ss;
            ss << "Sold " << qty << " " << ITEM_DEFS[selected].name << ".";
            log = ss.str();
        }
        save();
        show_market();
    }

    void deposit(long long amount) {
        if (cash <= 0) {
            log = "No positive cash to deposit.";
        } else {
            long long amt = std::min(amount, cash);
            cash -= amt;
            bank += amt;
            log = "Deposited " + money(amt) + ".";
        }
        save();
    }

    void withdraw(long long amount) {
        if (bank <= 0) {
            log = "No bank money to withdraw.";
        } else {
            long long amt = std::min(amount, bank);
            bank -= amt;
            cash += amt;
            log = "Withdrew " + money(amt) + ".";
        }
        save();
    }

    void pay_debt(long long amount) {
        if (cash <= 0) {
            log = "No positive cash to pay the loan shark.";
        } else if (debt <= 0) {
            log = "The loan shark is paid off.";
        } else {
            long long amt = std::min(amount, std::min(cash, debt));
            cash -= amt;
            debt -= amt;
            log = "Paid " + money(amt) + " to the loan shark.";
        }
        save();
    }

    void borrow(long long amount) {
        cash += amount;
        debt += amount;
        log = "Borrowed " + money(amount) + ".";
        save();
    }

    void accept_offer() {
        if (offer_type == 1) {
            if (cash >= GUN_PRICE) {
                cash -= GUN_PRICE;
                guns += 1;
                log = "Bought a .38.";
            } else {
                log = "Not enough cash for the gun.";
            }
        } else if (offer_type == 2) {
            if (cash >= COAT_PRICE) {
                cash -= COAT_PRICE;
                capacity += COAT_SIZE;
                log = "Bought the bigger trenchcoat. Capacity increased.";
            } else {
                log = "Not enough cash for the coat.";
            }
        }
        offer_type = 0;
        save();
        show_market();
    }

    void reject_offer() {
        offer_type = 0;
        log = "You walked away.";
        save();
        show_market();
    }

    void fight_cops() {
        if (cops <= 0) {
            show_market();
            return;
        }
        int killed = 0;
        if (guns > 0) {
            killed = rand_int(0, std::max(1, guns));
            killed = std::min(killed, cops);
            cops -= killed;
        }
        int damage = rand_int(4, 18) + std::max(0, cops - 2);
        if (cops > 0) health -= damage;
        std::ostringstream ss;
        if (guns <= 0) ss << "You have no gun. ";
        else ss << "You dropped " << killed << " cop" << (killed == 1 ? "" : "s") << ". ";
        if (cops > 0) ss << "They hit you for " << damage << ".";
        else ss << "You fought them off.";
        log = ss.str();
        if (health <= 0) {
            health = 0;
            end_game("You were killed by the police.");
            show_game_over();
            return;
        }
        if (cops <= 0) {
            cops = 0;
            save();
            show_market();
        } else {
            save();
            show_police();
        }
    }

    void run_from_cops() {
        if (cops <= 0) {
            show_market();
            return;
        }
        int chance = clampi(48 + guns * 5 - cops * 2, 25, 80);
        if (rand_int(1, 100) <= chance) {
            cops = 0;
            log = "You escaped.";
            save();
            show_market();
            return;
        }
        int damage = rand_int(8, 24) + cops;
        health -= damage;
        if (cash > 0 && rand_int(1, 100) <= 35) {
            long long fine = std::min<long long>(cash, rand_int(100, 1200));
            cash -= fine;
            log = "You failed to run. Took " + std::to_string(damage) + " damage and lost " + money(fine) + ".";
        } else {
            log = "You failed to run. Took " + std::to_string(damage) + " damage.";
        }
        if (health <= 0) {
            health = 0;
            end_game("You were killed while running from the police.");
            show_game_over();
            return;
        }
        save();
        show_police();
    }

    void save() const {
        std::ofstream out(save_path().c_str(), std::ios::trunc);
        if (!out) return;
        out << "day=" << day << "\n";
        out << "cash=" << cash << "\n";
        out << "debt=" << debt << "\n";
        out << "bank=" << bank << "\n";
        out << "health=" << health << "\n";
        out << "capacity=" << capacity << "\n";
        out << "guns=" << guns << "\n";
        out << "location=" << location << "\n";
        out << "selected=" << selected << "\n";
        out << "negative_days=" << negative_days << "\n";
        out << "future_event_day=" << future_event_day << "\n";
        out << "future_event_item=" << future_event_item << "\n";
        out << "future_event_type=" << future_event_type << "\n";
        out << "future_event_location=" << future_event_location << "\n";
        out << "future_event_known=" << (future_event_known ? 1 : 0) << "\n";
        out << "header_font=" << header_font << "\n";
        out << "info_font=" << info_font << "\n";
        out << "event_font=" << event_font << "\n";
        out << "chart_font=" << chart_font << "\n";
        out << "bottom_font=" << bottom_font << "\n";
        out << "section_gap=" << section_gap << "\n";
        out << "chart_gap=" << chart_gap << "\n";
        out << "bottom_gap=" << bottom_gap << "\n";
        out << "header_height_delta=" << header_height_delta << "\n";
        out << "stats_height_delta=" << stats_height_delta << "\n";
        out << "event_height_delta=" << event_height_delta << "\n";
        out << "chart_height_delta=" << chart_height_delta << "\n";
        out << "chart_row_delta=" << chart_row_delta << "\n";
        out << "bottom_height_delta=" << bottom_height_delta << "\n";
        out << "game_over=" << (game_over ? 1 : 0) << "\n";
        out << "reason=" << game_over_reason << "\n";
        for (int i = 0; i < ITEM_COUNT; ++i) out << "held" << i << "=" << held[i] << "\n";
        for (int i = 0; i < ITEM_COUNT; ++i) out << "price" << i << "=" << price[i] << "\n";
    }

    bool load() {
        std::ifstream in(save_path().c_str());
        if (!in) return false;
        std::map<std::string, std::string> kv;
        std::string line;
        while (std::getline(in, line)) {
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;
            kv[line.substr(0, pos)] = line.substr(pos + 1);
        }
        day = parse_int(kv["day"], 1);
        cash = parse_ll(kv["cash"], START_CASH);
        debt = parse_ll(kv["debt"], START_DEBT);
        bank = parse_ll(kv["bank"], 0);
        health = parse_int(kv["health"], START_HEALTH);
        capacity = parse_int(kv["capacity"], START_CAPACITY);
        guns = parse_int(kv["guns"], 0);
        location = clampi(parse_int(kv["location"], 0), 0, LOCATION_COUNT - 1);
        selected = clampi(parse_int(kv["selected"], 0), 0, ITEM_COUNT - 1);
        negative_days = parse_int(kv["negative_days"], 0);
        future_event_day = parse_int(kv["future_event_day"], 0);
        future_event_item = clampi(parse_int(kv["future_event_item"], 0), 0, ITEM_COUNT - 1);
        future_event_type = parse_int(kv["future_event_type"], 0);
        future_event_location = clampi(parse_int(kv["future_event_location"], 0), 0, LOCATION_COUNT - 1);
        future_event_known = parse_int(kv["future_event_known"], 0) != 0;
        header_font = clampi(parse_int(kv["header_font"], 9), 4, 18);
        info_font = clampi(parse_int(kv["info_font"], 11), 4, 22);
        event_font = clampi(parse_int(kv["event_font"], 14), 4, 26);
        chart_font = clampi(parse_int(kv["chart_font"], 6), 4, 18);
        bottom_font = clampi(parse_int(kv["bottom_font"], 9), 4, 22);
        section_gap = clampi(parse_int(kv["section_gap"], 0), -200, 400);
        chart_gap = clampi(parse_int(kv["chart_gap"], -8), -200, 400);
        bottom_gap = clampi(parse_int(kv["bottom_gap"], 0), -200, 400);
        header_height_delta = clampi(parse_int(kv["header_height_delta"], 0), -200, 400);
        stats_height_delta = clampi(parse_int(kv["stats_height_delta"], 0), -200, 400);
        event_height_delta = clampi(parse_int(kv["event_height_delta"], 0), -200, 400);
        chart_height_delta = clampi(parse_int(kv["chart_height_delta"], -80), -300, 500);
        chart_row_delta = clampi(parse_int(kv["chart_row_delta"], -10), -200, 400);
        bottom_height_delta = clampi(parse_int(kv["bottom_height_delta"], 0), -200, 400);
        if (future_event_day <= day || future_event_type < 1 || future_event_type > 2) schedule_future_event();
        game_over = parse_int(kv["game_over"], 0) != 0;
        game_over_reason = kv["reason"];
        for (int i = 0; i < ITEM_COUNT; ++i) held[i] = std::max(0, parse_int(kv["held" + std::to_string(i)], 0));
        bool valid_prices = true;
        for (int i = 0; i < ITEM_COUNT; ++i) {
            price[i] = parse_int(kv["price" + std::to_string(i)], 0);
            if (price[i] <= 0) valid_prices = false;
        }
        if (!valid_prices) generate_market();
        if (game_over_reason.empty()) game_over_reason = "The run has ended.";
        log = game_over ? "Saved game ended." : "Saved run loaded.";
        return true;
    }

    void apply_font(GtkWidget* w, int size = 10) {
        PangoFontDescription* desc = pango_font_description_from_string((std::string("Sans ") + std::to_string(size)).c_str());
        gtk_widget_modify_font(w, desc);
        pango_font_description_free(desc);
    }

    GtkWidget* label(const std::string& text, int size = 10, bool wrap = true,
                     float xalign = 0.0f, GtkJustification justify = GTK_JUSTIFY_LEFT) {
        GtkWidget* l = gtk_label_new(text.c_str());
        gtk_label_set_line_wrap(GTK_LABEL(l), wrap ? TRUE : FALSE);
        gtk_label_set_justify(GTK_LABEL(l), justify);
        gtk_label_set_line_wrap_mode(GTK_LABEL(l), PANGO_WRAP_WORD_CHAR);
        gtk_misc_set_alignment(GTK_MISC(l), xalign, 0.5f);
        apply_font(l, size);
        return l;
    }

    int compact_font_for(const std::string& text, int base = 10) const {
        if (text.size() > 22) return std::max(6, base - 4);
        if (text.size() > 18) return std::max(7, base - 3);
        if (text.size() > 14) return std::max(8, base - 2);
        if (text.size() > 11) return std::max(8, base - 1);
        return base;
    }

    GtkWidget* stat_label(const std::string& text) {
        GtkWidget* l = label(text, compact_font_for(text, header_font), false, 0.0f, GTK_JUSTIFY_LEFT);
        gtk_widget_set_size_request(l, 72, std::max(16, header_font + 10));
        return l;
    }

    GtkWidget* centered_label(const std::string& text, int size = 10, bool wrap = true) {
        return label(text, size, wrap, 0.5f, GTK_JUSTIFY_CENTER);
    }

    GtkWidget* button(const std::string& text, GCallback cb, gpointer data = nullptr, int size = 10,
                      int min_width = -1, int min_height = 28) {
        GtkWidget* b = gtk_button_new_with_label(text.c_str());
        apply_font(b, size);
        GtkWidget* child = gtk_bin_get_child(GTK_BIN(b));
        if (child && GTK_IS_LABEL(child)) {
            apply_font(child, size);
            gtk_misc_set_alignment(GTK_MISC(child), 0.5f, 0.5f);
            gtk_label_set_justify(GTK_LABEL(child), GTK_JUSTIFY_CENTER);
        }
        if (min_width > 0 || min_height > 0) gtk_widget_set_size_request(b, min_width, min_height);
        g_object_set_data(G_OBJECT(b), "app", this);
        g_signal_connect(b, "clicked", cb, data);
        return b;
    }

    GtkWidget* hline() {
        return gtk_hseparator_new();
    }

    void set_bg(GtkWidget* w, const char* color) {
        GdkColor bg;
        if (gdk_color_parse(color, &bg)) {
            gtk_widget_modify_bg(w, GTK_STATE_NORMAL, &bg);
            gtk_widget_modify_bg(w, GTK_STATE_ACTIVE, &bg);
            gtk_widget_modify_bg(w, GTK_STATE_PRELIGHT, &bg);
            gtk_widget_modify_bg(w, GTK_STATE_SELECTED, &bg);
        }
    }

    GtkWidget* region_wrap(GtkWidget* child, int min_height = -1) {
        GtkWidget* box = gtk_event_box_new();
        gtk_event_box_set_visible_window(GTK_EVENT_BOX(box), FALSE);
        if (min_height > 0) gtk_widget_set_size_request(box, -1, min_height);
        gtk_container_add(GTK_CONTAINER(box), child);
        return box;
    }

    GtkWidget* item_cell(const std::string& text, int index) {
        GtkWidget* eb = gtk_event_box_new();
        gtk_event_box_set_visible_window(GTK_EVENT_BOX(eb), FALSE);
        GtkWidget* l = centered_label(text, chart_font, false);
        gtk_container_add(GTK_CONTAINER(eb), l);
        g_object_set_data(G_OBJECT(eb), "app", this);
        g_object_set_data(G_OBJECT(eb), "item_index", GINT_TO_POINTER(index));
        g_signal_connect(eb, "button-press-event", G_CALLBACK(on_item_tap), nullptr);
        return eb;
    }

    void clear_root() {
        GList* children = gtk_container_get_children(GTK_CONTAINER(root));
        for (GList* it = children; it != nullptr; it = it->next) {
            gtk_widget_destroy(GTK_WIDGET(it->data));
        }
        g_list_free(children);
    }

    GtkWidget* vertical_spacer(int height) {
        GtkWidget* spacer = gtk_label_new("");
        gtk_widget_set_size_request(spacer, -1, height);
        return spacer;
    }

    int pack_gap() const {
        return std::max(0, section_gap);
    }

    int header_region_height(int font_size) const {
        return clampi(font_size + 24 + header_height_delta, 24, 110);
    }

    int stats_region_height() const {
        return clampi(info_font * 3 + 12 + stats_height_delta, 28, 140);
    }

    int event_region_height() const {
        return clampi(event_font * 2 + 12 + event_height_delta, 24, 130);
    }

    int chart_row_height() const {
        // This is the direct drug-row spacing control. Negative values shrink row height hard.
        return clampi(chart_font + 8 + chart_gap + chart_row_delta, 6, 96);
    }

    int chart_region_height() const {
        // Visible drug-table box height. Rows scroll inside this area when compacted.
        return clampi(190 + chart_height_delta, 70, 520);
    }

    int bottom_button_height() const {
        return clampi(bottom_font + 22 + bottom_gap + bottom_height_delta, 22, 90);
    }

    std::string stats_line() const {
        std::ostringstream ss;
        ss << "Day " << day << " | " << LOCATIONS[location]
           << " | Cash " << money(cash)
           << " | Debt " << money(debt)
           << " | Bank " << money(bank)
           << "\nHealth " << health
           << " | Guns " << guns
           << " | Coat " << used_space() << "/" << capacity
           << " | Negative days " << negative_days << "/" << NEGATIVE_LIMIT;
        return ss.str();
    }

    std::string event_text() const {
        std::ostringstream ss;
        ss << log;
        if (future_event_known && !game_over) {
            ss << "\n" << future_tip_text();
        } else if (!game_over) {
            ss << "\nTap this news area to buy street information (" << money(tip_cost()) << ").";
        }
        return ss.str();
    }

    void export_ui_settings() {
        std::ofstream out(ui_debug_path().c_str(), std::ios::trunc);
        if (out) {
            out << "Kindle Dope Wars UI debug export\n";
            out << "day=" << day << "\n";
            out << "header_font=" << header_font << "\n";
            out << "stats_font=" << info_font << "\n";
            out << "event_font=" << event_font << "\n";
            out << "chart_font=" << chart_font << "\n";
            out << "bottom_font=" << bottom_font << "\n";
            out << "section_gap=" << section_gap << "\n";
            out << "chart_gap=" << chart_gap << "\n";
            out << "bottom_gap=" << bottom_gap << "\n";
            out << "header_height_delta=" << header_height_delta << "\n";
            out << "stats_height_delta=" << stats_height_delta << "\n";
            out << "event_height_delta=" << event_height_delta << "\n";
            out << "chart_height_delta=" << chart_height_delta << "\n";
            out << "chart_row_delta=" << chart_row_delta << "\n";
            out << "bottom_height_delta=" << bottom_height_delta << "\n";
            out << "effective_header_height=" << header_region_height(clampi(header_font + 2, 10, 16)) << "\n";
            out << "effective_stats_height=" << stats_region_height() << "\n";
            out << "effective_event_height=" << event_region_height() << "\n";
            out << "effective_chart_region_height=" << chart_region_height() << "\n";
            out << "effective_chart_row_height=" << chart_row_height() << "\n";
            out << "effective_bottom_button_height=" << bottom_button_height() << "\n";
        }
        log = "UI debug exported to " + ui_debug_path() + ".";
        save();
    }

    void adjust_font_setting(int code) {
        int sign = code < 0 ? -1 : 1;
        int target = std::abs(code);
        int font_delta = sign;
        int layout_delta = sign * 5;

        if (target == 1) header_font = clampi(header_font + font_delta, 3, 30);
        else if (target == 2) info_font = clampi(info_font + font_delta, 3, 34);
        else if (target == 3) event_font = clampi(event_font + font_delta, 3, 38);
        else if (target == 4) chart_font = clampi(chart_font + font_delta, 3, 30);
        else if (target == 5) bottom_font = clampi(bottom_font + font_delta, 3, 34);
        else if (target == 6) section_gap = clampi(section_gap + layout_delta, -200, 400);
        else if (target == 7) chart_gap = clampi(chart_gap + layout_delta, -200, 400);
        else if (target == 8) bottom_gap = clampi(bottom_gap + layout_delta, -200, 400);
        else if (target == 9) header_height_delta = clampi(header_height_delta + layout_delta, -200, 400);
        else if (target == 10) stats_height_delta = clampi(stats_height_delta + layout_delta, -200, 400);
        else if (target == 11) event_height_delta = clampi(event_height_delta + layout_delta, -200, 400);
        else if (target == 12) chart_height_delta = clampi(chart_height_delta + layout_delta, -300, 500);
        else if (target == 13) chart_row_delta = clampi(chart_row_delta + layout_delta, -200, 400);
        else if (target == 14) bottom_height_delta = clampi(bottom_height_delta + layout_delta, -200, 400);
        export_ui_settings();
        save();
    }

    enum DeferredKind {
        ACT_SELECT_ITEM = 1,
        ACT_BUY,
        ACT_SELL,
        ACT_SHOW_TRAVEL,
        ACT_SHOW_BANK,
        ACT_SHOW_SETTINGS,
        ACT_ADJUST_FONT,
        ACT_SHOW_MARKET,
        ACT_SHOW_TIP_CONFIRM,
        ACT_BUY_NEWS,
        ACT_TRAVEL,
        ACT_DEPOSIT,
        ACT_WITHDRAW,
        ACT_PAY_DEBT,
        ACT_BORROW,
        ACT_FIGHT,
        ACT_RUN,
        ACT_ACCEPT_OFFER,
        ACT_REJECT_OFFER,
        ACT_CONFIRM_NEW,
        ACT_NEW_GAME,
        ACT_EXPORT_UI
    };

    struct DeferredAction {
        GameApp* app;
        int kind;
        int value;
    };

    void queue_action(int kind, int value = 0) {
        DeferredAction* action = new DeferredAction{this, kind, value};
        g_timeout_add(90, GameApp::run_deferred_action, action);
    }

    static gboolean run_deferred_action(gpointer data) {
        DeferredAction* action = static_cast<DeferredAction*>(data);
        GameApp* app = action ? action->app : nullptr;
        int kind = action ? action->kind : 0;
        int value = action ? action->value : 0;
        delete action;
        if (!app) return FALSE;

        switch (kind) {
            case ACT_SELECT_ITEM:
                app->selected = clampi(value, 0, ITEM_COUNT - 1);
                app->save();
                app->show_market();
                break;
            case ACT_BUY:
                app->buy_selected(value);
                break;
            case ACT_SELL:
                app->sell_selected(value);
                break;
            case ACT_SHOW_TRAVEL:
                app->show_travel();
                break;
            case ACT_SHOW_BANK:
                app->show_bank();
                break;
            case ACT_SHOW_SETTINGS:
                app->show_settings();
                break;
            case ACT_ADJUST_FONT:
                app->adjust_font_setting(value);
                app->show_settings();
                break;
            case ACT_SHOW_MARKET:
                app->show_market();
                break;
            case ACT_SHOW_TIP_CONFIRM:
                app->show_tip_confirm();
                break;
            case ACT_BUY_NEWS:
                app->buy_street_tip();
                break;
            case ACT_TRAVEL:
                app->travel_to(value);
                break;
            case ACT_DEPOSIT:
                app->deposit(value >= 999999 ? std::max<long long>(0, app->cash) : value);
                app->show_market();
                break;
            case ACT_WITHDRAW:
                app->withdraw(value >= 999999 ? std::max<long long>(0, app->bank) : value);
                app->show_market();
                break;
            case ACT_PAY_DEBT:
                app->pay_debt(value >= 999999 ? std::max<long long>(0, std::min(app->cash, app->debt)) : value);
                app->show_market();
                break;
            case ACT_BORROW:
                app->borrow(value);
                app->show_market();
                break;
            case ACT_FIGHT:
                app->fight_cops();
                break;
            case ACT_RUN:
                app->run_from_cops();
                break;
            case ACT_ACCEPT_OFFER:
                app->accept_offer();
                break;
            case ACT_REJECT_OFFER:
                app->reject_offer();
                break;
            case ACT_CONFIRM_NEW:
                app->show_new_confirm();
                break;
            case ACT_NEW_GAME:
                app->new_game();
                app->show_market();
                break;
            case ACT_EXPORT_UI:
                app->export_ui_settings();
                app->show_settings();
                break;
            default:
                break;
        }
        return FALSE;
    }

    void show_market() {
        if (game_over) {
            show_game_over();
            return;
        }
        cops = 0;
        offer_type = 0;
        clear_root();

        int gap = pack_gap();
        int top_font = clampi(header_font + 2, 10, 16);
        int top_h = header_region_height(top_font);
        GtkWidget* top_row = gtk_hbox_new(FALSE, std::max(1, gap));
        gtk_container_set_border_width(GTK_CONTAINER(top_row), std::max(1, gap));
        gtk_box_pack_start(GTK_BOX(top_row), stat_label("Day: " + std::to_string(day)), TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(top_row), stat_label("Health: " + std::to_string(health)), TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(top_row), stat_label("Coat: " + std::to_string(used_space()) + "/" + std::to_string(capacity)), TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(top_row), stat_label("Bank: " + money(bank)), TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(top_row), button("New", G_CALLBACK(on_confirm_new), nullptr, top_font, 60, top_h), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(top_row), button("Settings", G_CALLBACK(on_show_settings), nullptr, std::max(9, top_font - 1), 108, top_h), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(top_row), button("Exit", G_CALLBACK(on_exit), nullptr, top_font, 60, top_h), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(root), region_wrap(top_row, top_h), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(root), hline(), FALSE, FALSE, gap);

        GtkWidget* summary = gtk_hbox_new(FALSE, 3 + gap);
        gtk_container_set_border_width(GTK_CONTAINER(summary), 2 + gap);
        GtkWidget* money_box = gtk_vbox_new(FALSE, 1);
        gtk_box_pack_start(GTK_BOX(money_box), label("Cash: " + money(cash), compact_font_for("Cash: " + money(cash), info_font), false), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(money_box), label("Debt: " + money(debt), compact_font_for("Debt: " + money(debt), info_font), false), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(money_box), label("Debt Days: " + std::to_string(negative_days) + "/" + std::to_string(NEGATIVE_LIMIT), info_font, false), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(summary), money_box, FALSE, FALSE, 2 + gap);
        gtk_box_pack_start(GTK_BOX(summary), gtk_vseparator_new(), FALSE, FALSE, 3 + gap);
        GtkWidget* loc = centered_label(LOCATIONS[location], compact_font_for(LOCATIONS[location], clampi(info_font + 8, 12, 24)), false);
        gtk_box_pack_start(GTK_BOX(summary), loc, TRUE, TRUE, 1 + gap);
        gtk_box_pack_start(GTK_BOX(root), region_wrap(summary, stats_region_height()), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(root), hline(), FALSE, FALSE, gap);

        GtkWidget* news_box = gtk_event_box_new();
        gtk_event_box_set_visible_window(GTK_EVENT_BOX(news_box), TRUE);
        gtk_widget_set_size_request(news_box, -1, event_region_height());
        GtkWidget* news_label = label(event_text(), event_font, true, 0.5f, GTK_JUSTIFY_CENTER);
        gtk_container_add(GTK_CONTAINER(news_box), news_label);
        g_object_set_data(G_OBJECT(news_box), "app", this);
        g_signal_connect(news_box, "button-press-event", G_CALLBACK(on_news_tap), nullptr);
        gtk_box_pack_start(GTK_BOX(root), news_box, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(root), hline(), FALSE, FALSE, gap);

        GtkWidget* scroller = gtk_scrolled_window_new(nullptr, nullptr);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        GtkWidget* table = gtk_table_new(ITEM_COUNT + 1, 3, FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table), 0);
        gtk_table_set_col_spacings(GTK_TABLE(table), 1 + gap);
        int chart_row_h = chart_row_height();
        gtk_table_attach_defaults(GTK_TABLE(table), centered_label("Item", chart_font, false), 0, 1, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(table), centered_label("Price", chart_font, false), 1, 2, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(table), centered_label("Held", chart_font, false), 2, 3, 0, 1);
        for (int i = 0; i < ITEM_COUNT; ++i) {
            std::string name = (i == selected ? "> " : "") + std::string(ITEM_DEFS[i].name);
            GtkWidget* item = item_cell(name, i);
            gtk_widget_set_size_request(item, -1, chart_row_h);
            gtk_table_attach_defaults(GTK_TABLE(table), item, 0, 1, i + 1, i + 2);
            gtk_table_attach_defaults(GTK_TABLE(table), centered_label(money(price[i]), compact_font_for(money(price[i]), chart_font), false), 1, 2, i + 1, i + 2);
            gtk_table_attach_defaults(GTK_TABLE(table), centered_label(std::to_string(held[i]), chart_font, false), 2, 3, i + 1, i + 2);
        }
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), table);
        gtk_box_pack_start(GTK_BOX(root), region_wrap(scroller, chart_region_height()), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(root), hline(), FALSE, FALSE, gap);

        int bottom_h = bottom_button_height();
        GtkWidget* bottom_box = gtk_vbox_new(FALSE, bottom_gap);
        gtk_container_set_border_width(GTK_CONTAINER(bottom_box), 2 + bottom_gap);
        GtkWidget* buy_row = gtk_hbox_new(TRUE, 3 + bottom_gap);
        gtk_box_pack_start(GTK_BOX(buy_row), button("Buy 1", G_CALLBACK(on_buy), GINT_TO_POINTER(1), bottom_font, -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(buy_row), button("Buy 10", G_CALLBACK(on_buy), GINT_TO_POINTER(10), bottom_font, -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(buy_row), button("Buy Max", G_CALLBACK(on_buy), GINT_TO_POINTER(9999), std::max(5, bottom_font - 1), -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(buy_row), button("Travel", G_CALLBACK(on_show_travel), nullptr, clampi(bottom_font + 4, 8, 24), -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(bottom_box), buy_row, FALSE, FALSE, 0);

        GtkWidget* sell_row = gtk_hbox_new(TRUE, 3 + bottom_gap);
        gtk_box_pack_start(GTK_BOX(sell_row), button("Sell 1", G_CALLBACK(on_sell), GINT_TO_POINTER(1), bottom_font, -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(sell_row), button("Sell 10", G_CALLBACK(on_sell), GINT_TO_POINTER(10), bottom_font, -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(sell_row), button("Sell Max", G_CALLBACK(on_sell), GINT_TO_POINTER(9999), std::max(5, bottom_font - 1), -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(sell_row), button("Bank", G_CALLBACK(on_show_bank), nullptr, clampi(bottom_font + 4, 8, 24), -1, bottom_h), TRUE, TRUE, bottom_gap);
        gtk_box_pack_start(GTK_BOX(bottom_box), sell_row, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(root), region_wrap(bottom_box, clampi(bottom_h * 2 + std::max(0, bottom_gap) * 5 + 6, 48, 220)), FALSE, FALSE, 0);
        gtk_widget_show_all(window);
    }

    void show_tip_confirm() {
        GtkWidget* dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Street Tip");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
        GtkWidget* area = GTK_DIALOG(dialog)->vbox;
        std::ostringstream ss;
        ss << "Buy street information for " << money(tip_cost()) << "?\n"
           << "Tips reveal a future shortage or flood. Tip prices rise by $50 each day.";
        gtk_box_pack_start(GTK_BOX(area), centered_label("Street Tip", 22, false), FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(area), label(ss.str(), 15, true, 0.5f, GTK_JUSTIFY_CENTER), FALSE, FALSE, 8);
        GtkWidget* row = gtk_hbox_new(TRUE, 10);
        GtkWidget* yes = button("Yes", G_CALLBACK(on_confirm_tip_yes), nullptr, 20, 150, 56);
        GtkWidget* no = button("No", G_CALLBACK(on_close_dialog), nullptr, 20, 150, 56);
        g_object_set_data(G_OBJECT(yes), "dialog", dialog);
        g_object_set_data(G_OBJECT(no), "dialog", dialog);
        gtk_box_pack_start(GTK_BOX(row), yes, TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(row), no, TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(area), row, FALSE, FALSE, 8);
        gtk_widget_show_all(dialog);
    }

    void show_travel() {
        GtkWidget* dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Travel");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 8);
        GtkWidget* area = GTK_DIALOG(dialog)->vbox;
        gtk_box_pack_start(GTK_BOX(area), centered_label("Travel", 24, false), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(area), label("Travel advances one day. Debt gains 10% interest.", 15, true), FALSE, FALSE, 4);
        for (int i = 0; i < LOCATION_COUNT; ++i) {
            std::string text = std::string(LOCATIONS[i]) + (i == location ? " (current)" : "");
            GtkWidget* b = button(text, G_CALLBACK(on_travel), GINT_TO_POINTER(i), 18, 320, 42);
            g_object_set_data(G_OBJECT(b), "dialog", dialog);
            gtk_box_pack_start(GTK_BOX(area), b, FALSE, FALSE, 3);
        }
        GtkWidget* close = button("Close", G_CALLBACK(on_close_dialog), nullptr, 18, 320, 42);
        g_object_set_data(G_OBJECT(close), "dialog", dialog);
        gtk_box_pack_start(GTK_BOX(area), close, FALSE, FALSE, 6);
        gtk_widget_show_all(dialog);
    }

    void show_bank() {
        GtkWidget* dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Bank");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 8);
        GtkWidget* area = GTK_DIALOG(dialog)->vbox;
        gtk_box_pack_start(GTK_BOX(area), centered_label("Bank / Loan Shark", 22, false), FALSE, FALSE, 3);
        gtk_box_pack_start(GTK_BOX(area), label("Cash: " + money(cash) + "\nBank: " + money(bank) + "\nDebt: " + money(debt), 16, true), FALSE, FALSE, 4);

        GtkWidget* dep = gtk_hbox_new(TRUE, 5);
        GtkWidget* b1 = button("Deposit $1k", G_CALLBACK(on_deposit), GINT_TO_POINTER(1000), 14, -1, 42);
        GtkWidget* b2 = button("Deposit All", G_CALLBACK(on_deposit), GINT_TO_POINTER(999999), 14, -1, 42);
        g_object_set_data(G_OBJECT(b1), "dialog", dialog);
        g_object_set_data(G_OBJECT(b2), "dialog", dialog);
        gtk_box_pack_start(GTK_BOX(dep), b1, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(dep), b2, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(area), dep, FALSE, FALSE, 2);

        GtkWidget* wit = gtk_hbox_new(TRUE, 5);
        GtkWidget* b3 = button("Withdraw $1k", G_CALLBACK(on_withdraw), GINT_TO_POINTER(1000), 13, -1, 42);
        GtkWidget* b4 = button("Withdraw All", G_CALLBACK(on_withdraw), GINT_TO_POINTER(999999), 13, -1, 42);
        g_object_set_data(G_OBJECT(b3), "dialog", dialog);
        g_object_set_data(G_OBJECT(b4), "dialog", dialog);
        gtk_box_pack_start(GTK_BOX(wit), b3, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(wit), b4, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(area), wit, FALSE, FALSE, 2);

        GtkWidget* debt_row = gtk_hbox_new(TRUE, 5);
        GtkWidget* b5 = button("Pay $1k", G_CALLBACK(on_pay_debt), GINT_TO_POINTER(1000), 14, -1, 42);
        GtkWidget* b6 = button("Pay Max", G_CALLBACK(on_pay_debt), GINT_TO_POINTER(999999), 14, -1, 42);
        GtkWidget* b7 = button("Borrow $1k", G_CALLBACK(on_borrow), GINT_TO_POINTER(1000), 14, -1, 42);
        g_object_set_data(G_OBJECT(b5), "dialog", dialog);
        g_object_set_data(G_OBJECT(b6), "dialog", dialog);
        g_object_set_data(G_OBJECT(b7), "dialog", dialog);
        gtk_box_pack_start(GTK_BOX(debt_row), b5, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(debt_row), b6, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(debt_row), b7, TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(area), debt_row, FALSE, FALSE, 2);

        GtkWidget* close = button("Close", G_CALLBACK(on_close_dialog), nullptr, 18, 360, 42);
        g_object_set_data(G_OBJECT(close), "dialog", dialog);
        gtk_box_pack_start(GTK_BOX(area), close, FALSE, FALSE, 6);
        gtk_widget_show_all(dialog);
    }

    void show_settings() {
        clear_root();
        gtk_box_pack_start(GTK_BOX(root), centered_label("Settings / UI Debug", 18, false), FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(root), centered_label("Font = +/-1. Layout/spacing = +/-5. Layout export writes ui-debug.txt.", 9, true), FALSE, FALSE, 1);

        GtkWidget* scroller = gtk_scrolled_window_new(nullptr, nullptr);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        GtkWidget* panel = gtk_vbox_new(FALSE, 1);
        gtk_container_set_border_width(GTK_CONTAINER(panel), 2);

        auto add_debug_row = [&](const std::string& name, int value, int code) {
            GtkWidget* row = gtk_hbox_new(FALSE, 4);
            GtkWidget* minus = button("-", G_CALLBACK(on_adjust_font), GINT_TO_POINTER(-code), 18, 72, 38);
            GtkWidget* mid = centered_label(name + ": " + std::to_string(value), 11, false);
            GtkWidget* plus = button("+", G_CALLBACK(on_adjust_font), GINT_TO_POINTER(code), 18, 72, 38);
            gtk_widget_set_size_request(mid, 270, 38);
            gtk_box_pack_start(GTK_BOX(row), minus, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(row), mid, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(row), plus, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(panel), row, FALSE, FALSE, 1);
        };

        gtk_box_pack_start(GTK_BOX(panel), centered_label("Text size", 12, false), FALSE, FALSE, 2);
        add_debug_row("header text", header_font, 1);
        add_debug_row("stats text", info_font, 2);
        add_debug_row("event text", event_font, 3);
        add_debug_row("chart text", chart_font, 4);
        add_debug_row("bottom text", bottom_font, 5);

        gtk_box_pack_start(GTK_BOX(panel), hline(), FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(panel), centered_label("Layout / spacing", 12, false), FALSE, FALSE, 2);
        add_debug_row("section gap", section_gap, 6);
        add_debug_row("chart cell gap", chart_gap, 7);
        add_debug_row("bottom gap", bottom_gap, 8);
        add_debug_row("header height", header_height_delta, 9);
        add_debug_row("stats height", stats_height_delta, 10);
        add_debug_row("event height", event_height_delta, 11);
        add_debug_row("chart box height", chart_height_delta, 12);
        add_debug_row("drug row height", chart_row_delta, 13);
        add_debug_row("bottom height", bottom_height_delta, 14);

        gtk_box_pack_start(GTK_BOX(panel), label("Drug row height directly controls spacing between buyable drug rows. Chart box height controls how much of the screen the whole drug chart consumes. Negative values are allowed; effective pixel heights are clamped only to prevent GTK from creating unusable zero-height widgets.", 9, true, 0.5f, GTK_JUSTIFY_CENTER), FALSE, FALSE, 3);

        GtkWidget* export_row = gtk_hbox_new(TRUE, 5);
        gtk_box_pack_start(GTK_BOX(export_row), button("Export UI", G_CALLBACK(on_export_ui), nullptr, 14, -1, 42), TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(export_row), button("Back", G_CALLBACK(on_show_market), nullptr, 14, -1, 42), TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(panel), export_row, FALSE, FALSE, 3);

        GtkWidget* main_row = gtk_hbox_new(TRUE, 5);
        gtk_box_pack_start(GTK_BOX(main_row), button("New Run", G_CALLBACK(on_confirm_new), nullptr, 14, -1, 42), TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(main_row), button("Exit", G_CALLBACK(on_exit), nullptr, 14, -1, 42), TRUE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX(panel), main_row, FALSE, FALSE, 3);

        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), panel);
        gtk_box_pack_start(GTK_BOX(root), scroller, TRUE, TRUE, 0);
        gtk_widget_show_all(window);
    }

    void show_police() {
        clear_root();
        gtk_box_pack_start(GTK_BOX(root), label("Police", 24, false), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(root), label(stats_line(), 15, true), FALSE, FALSE, 4);
        std::ostringstream ss;
        ss << log << "\n" << cops << " cop" << (cops == 1 ? " is" : "s are") << " on you.";
        gtk_box_pack_start(GTK_BOX(root), label(ss.str(), 18, true), FALSE, FALSE, 8);
        GtkWidget* row = gtk_hbox_new(TRUE, 8);
        gtk_box_pack_start(GTK_BOX(row), button("Fight", G_CALLBACK(on_fight), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(row), button("Run", G_CALLBACK(on_run), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(root), row, FALSE, FALSE, 12);
        gtk_widget_show_all(window);
    }

    void show_offer() {
        clear_root();
        gtk_box_pack_start(GTK_BOX(root), label("Street Offer", 24, false), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(root), label(stats_line(), 15, true), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(root), label(log, 18, true), FALSE, FALSE, 10);
        GtkWidget* row = gtk_hbox_new(TRUE, 8);
        gtk_box_pack_start(GTK_BOX(row), button("Buy", G_CALLBACK(on_accept_offer), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(row), button("Pass", G_CALLBACK(on_reject_offer), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(root), row, FALSE, FALSE, 12);
        gtk_widget_show_all(window);
    }

    void show_new_confirm() {
        clear_root();
        gtk_box_pack_start(GTK_BOX(root), label("Start New Run?", 24, false), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(root), label("This will overwrite the saved run.", 17, true), FALSE, FALSE, 8);
        GtkWidget* row = gtk_hbox_new(TRUE, 8);
        gtk_box_pack_start(GTK_BOX(row), button("Start New", G_CALLBACK(on_new_game), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(row), button("Cancel", G_CALLBACK(on_show_market), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(root), row, FALSE, FALSE, 12);
        gtk_widget_show_all(window);
    }

    void show_game_over() {
        clear_root();
        gtk_box_pack_start(GTK_BOX(root), label("Game Over", 26, false), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(root), label(game_over_reason, 18, true), FALSE, FALSE, 6);
        gtk_box_pack_start(GTK_BOX(root), label(stats_line(), 15, true), FALSE, FALSE, 4);
        gtk_box_pack_start(GTK_BOX(root), label("Final net worth: " + money(net_worth()), 20, true), FALSE, FALSE, 10);
        GtkWidget* row = gtk_hbox_new(TRUE, 8);
        gtk_box_pack_start(GTK_BOX(row), button("New Run", G_CALLBACK(on_new_game), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(row), button("Exit", G_CALLBACK(on_exit), nullptr, 20), TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(root), row, FALSE, FALSE, 12);
        gtk_widget_show_all(window);
    }

    void build_window(int* argc, char*** argv) {
        gtk_init(argc, argv);

        // Use a normal GTK toplevel for stable touch/click handling, then force it fullscreen and above Home.
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        g_object_set_data(G_OBJECT(window), "app", this);
        gtk_window_set_title(GTK_WINDOW(window), "L:A_N:application_PC:N_ID:kindledopewars");
        gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);

        int screen_w = gdk_screen_width();
        int screen_h = gdk_screen_height();
        if (screen_w < 300 || screen_h < 300) {
            screen_w = 758;
            screen_h = 1024;
        }
        gtk_window_move(GTK_WINDOW(window), 0, 0);
        gtk_widget_set_size_request(window, screen_w, screen_h);
        gtk_window_set_default_size(GTK_WINDOW(window), screen_w, screen_h);
        gtk_window_fullscreen(GTK_WINDOW(window));
        gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

        g_signal_connect(window, "destroy", G_CALLBACK(on_exit), nullptr);
        root = gtk_vbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(root), 1);
        gtk_container_add(GTK_CONTAINER(window), root);
        if (!load()) new_game();
        if (game_over) show_game_over();
        else show_market();
        gtk_window_present(GTK_WINDOW(window));
    }

    static GameApp* app_from_widget(GtkWidget* w) {
        return static_cast<GameApp*>(g_object_get_data(G_OBJECT(w), "app"));
    }

    static void on_select_item(GtkWidget* w, gpointer data) {
        app_from_widget(w)->queue_action(ACT_SELECT_ITEM, GPOINTER_TO_INT(data));
    }

    static void on_buy(GtkWidget* w, gpointer data) {
        app_from_widget(w)->queue_action(ACT_BUY, GPOINTER_TO_INT(data));
    }

    static void on_sell(GtkWidget* w, gpointer data) {
        app_from_widget(w)->queue_action(ACT_SELL, GPOINTER_TO_INT(data));
    }

    static void on_show_travel(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_SHOW_TRAVEL);
    }

    static void on_show_bank(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_SHOW_BANK);
    }

    static void on_show_settings(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_SHOW_SETTINGS);
    }

    static void on_adjust_font(GtkWidget* w, gpointer data) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        app->queue_action(ACT_ADJUST_FONT, GPOINTER_TO_INT(data));
    }

    static void on_show_market(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_SHOW_MARKET);
    }

    static gboolean on_news_tap(GtkWidget* w, GdkEventButton*, gpointer) {
        app_from_widget(w)->queue_action(ACT_SHOW_TIP_CONFIRM);
        return TRUE;
    }

    static gboolean on_item_tap(GtkWidget* w, GdkEventButton*, gpointer) {
        GameApp* app = app_from_widget(w);
        int index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "item_index"));
        if (app) app->queue_action(ACT_SELECT_ITEM, index);
        return TRUE;
    }

    static void on_confirm_tip_yes(GtkWidget* w, gpointer) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        if (app) app->queue_action(ACT_BUY_NEWS);
    }

    static void close_parent_dialog(GtkWidget* w) {
        GtkWidget* dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(w), "dialog"));
        if (dialog) gtk_widget_destroy(dialog);
    }

    static void on_close_dialog(GtkWidget* w, gpointer) {
        close_parent_dialog(w);
    }

    static void on_travel(GtkWidget* w, gpointer data) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        app->queue_action(ACT_TRAVEL, GPOINTER_TO_INT(data));
    }

    static void on_deposit(GtkWidget* w, gpointer data) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        app->queue_action(ACT_DEPOSIT, GPOINTER_TO_INT(data));
    }

    static void on_withdraw(GtkWidget* w, gpointer data) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        app->queue_action(ACT_WITHDRAW, GPOINTER_TO_INT(data));
    }

    static void on_pay_debt(GtkWidget* w, gpointer data) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        app->queue_action(ACT_PAY_DEBT, GPOINTER_TO_INT(data));
    }

    static void on_borrow(GtkWidget* w, gpointer data) {
        GameApp* app = app_from_widget(w);
        close_parent_dialog(w);
        app->queue_action(ACT_BORROW, GPOINTER_TO_INT(data));
    }

    static void on_fight(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_FIGHT);
    }

    static void on_run(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_RUN);
    }

    static void on_accept_offer(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_ACCEPT_OFFER);
    }

    static void on_reject_offer(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_REJECT_OFFER);
    }

    static void on_confirm_new(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_CONFIRM_NEW);
    }

    static void on_new_game(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_NEW_GAME);
    }

    static void on_export_ui(GtkWidget* w, gpointer) {
        app_from_widget(w)->queue_action(ACT_EXPORT_UI);
    }

    static void on_exit(GtkWidget* w, gpointer) {
        GameApp* app = nullptr;
        if (w && G_IS_OBJECT(w)) app = static_cast<GameApp*>(g_object_get_data(G_OBJECT(w), "app"));
        if (app) app->save();
        gtk_main_quit();
    }
};

} // namespace

int main(int argc, char** argv) {
    GameApp app;
    app.build_window(&argc, &argv);
    gtk_main();
    return 0;
}
