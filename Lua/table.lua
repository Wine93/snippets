-- Copyright (C) 2019, by Yuyang Chen (Wine93)


-- The performance test for lua table(array) appending elements

-- sample output:
--[[
    TEST 1.0: count=10000000, alloc=normal, append=*
    ==================================================
    [√]  normal#record: 0.10 (s) = 0.00 + 0.10
    [√]  normal#normal: 1.87 (s) = 0.00 + 1.87
    [√]  normal#insert: 1.92 (s) = 0.00 + 1.92


    TEST 2.0: count=10000000, alloc=*, append=record
    ==================================================
    [√]  preset#record: 0.06 (s) = 0.04 + 0.02
    [√]  normal#record: 0.10 (s) = 0.00 + 0.10
    [√]    grow#record: 0.14 (s) = 0.00 + 0.14


    TEST 3.0: count=10000000, alloc=*, append=*
    ==================================================
    [√]  preset#record: 0.06 (s) = 0.04 + 0.02
    [√]  preset#normal: 1.26 (s) = 0.04 + 1.22
    [√]  preset#insert: 1.36 (s) = 0.03 + 1.33
    [√]  normal#record: 0.10 (s) = 0.00 + 0.10
    [√]  normal#normal: 1.84 (s) = 0.00 + 1.84
    [√]  normal#insert: 1.94 (s) = 0.00 + 1.94
    [√]    grow#record: 0.13 (s) = 0.00 + 0.13
    [√]    grow#normal: 1.71 (s) = 0.00 + 1.71
    [√]    grow#insert: 1.77 (s) = 0.00 + 1.77
--]]


-- API
local os_clock = os.clock
local tab_insert = table.insert
local setmetatable = setmetatable

local succ, new_tab = pcall(require, "table.new")
if not succ then
    error ("please use luajit(>=2.1) to run it!")
end

-- CONST
local ALLOC = {
    NORMAL = "normal", -- local array = {}
    PRESET = "preset", -- local array = new_tab(capacity, 0)
    GROW   = "grow",   -- local array = new_tab([1, 2, 4, 8...], 0)
}

local APPEND = {
    NORMAL = "normal", -- array[#array+1] = value
    RECORD = "record", -- array[++size] = value
    INSERT = "insert", -- table.insert(array, value)
}

local INF_CAPACITY = 2^32

local SEP = string.rep("=", 50)
local TAB = string.rep(" ", 4)
local SIGN_RES = {
    [true] = "[√]",
    [false] = "[✘]",
}


-- tester
local tester = { _VERSION = "0.01" }
local mt = { __index = tester }


function tester.new(_, count, alloc, append)
    local start_sec = os_clock()

    -- array, capacity
    local array
    local capacity
    if alloc == ALLOC.NORMAL then
        array = {}
        capacity = INF_CAPACITY
    elseif alloc == ALLOC.PRESET then
        array = new_tab(count, 0)
        capacity = count
    elseif alloc == ALLOC.GROW then
        array = new_tab(1, 0)
        capacity = 1
    end

    local end_sec = os_clock()

    local self = {
        count = count,
        alloc = alloc,
        append = append,
        array = array,
        capacity = capacity,
        create_sec = end_sec - start_sec,
        size = 0,  -- only for record append
    }
    return setmetatable(self, mt)
end


function tester.run(self)
    self.start_sec = os_clock()

    local count = self.count
    local append = self.append

    for i = 1, count do
        if i > self.capacity then
            self:grow_array()
        end

        if append == APPEND.NORMAL then
            self.array[#self.array + 1] = i
        elseif append == APPEND.RECORD then
            self.size = self.size + 1
            self.array[self.size] = i
        elseif append == APPEND.INSERT then
            tab_insert(self.array, i)
        end
    end

    self.end_sec = os_clock()
end


function tester.grow_array(self)
    local new_capacity = self.capacity * 2
    if new_capacity > self.count then
        new_capacity = self.count
    end

    local array = self.array
    local new_array = new_tab(new_capacity, 0)
    local size = #self.array
    for i = 1, size do
        new_array[i] = array[i]
    end

    self.capacity = new_capacity
    self.array = new_array
end


function tester.check(self)
    local count = self.count
    local array = self.array

    for i = 1, count do
        if array[i] ~= i then
            return false
        end
    end

    return true
end


local function TEST(des, count, allocs, appends)
    print (("%s\n%s"):format(des, SEP))

    for _, alloc in ipairs(allocs) do
        for _, append in ipairs(appends) do
            local t = tester:new(count, alloc, append)

            t:run()

            local item = alloc .. "#" .. append

            local create_sec = t.create_sec
            local run_sec = t.end_sec - t.start_sec
            local total_sec = create_sec + run_sec

            local ret = t:check()

            print (("%s %14s: %.2f (s) = %.2f + %.2f"):format(
                SIGN_RES[ret], item, total_sec, create_sec, run_sec)
            )

            -- Ensure alloc memory don't waste time
            t = nil
            collectgarbage()
        end
    end

    print ("\n")
end


TEST(
    "TEST 1.0: count=10000000, alloc=normal, append=*",
    10000000,
    { ALLOC.NORMAL },
    { APPEND.RECORD, APPEND.NORMAL, APPEND.INSERT }
)


TEST(
    "TEST 2.0: count=10000000, alloc=*, append=record",
    10000000,
    { ALLOC.PRESET, ALLOC.NORMAL, ALLOC.GROW },
    { APPEND.RECORD }
)


TEST(
    "TEST 3.0: count=10000000, alloc=*, append=*",
    10000000,
    { ALLOC.PRESET, ALLOC.NORMAL, ALLOC.GROW },
    { APPEND.RECORD, APPEND.NORMAL, APPEND.INSERT }
)
