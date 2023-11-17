
.. _program_listing_file__mnt_c_Users_scott_OneDrive_Desktop_New_folder_New_folder_README.md:

Program Listing for File README.md
==================================

|exhale_lsh| :ref:`Return to documentation for file <file__mnt_c_Users_scott_OneDrive_Desktop_New_folder_New_folder_README.md>` (``/mnt/c/Users/scott/OneDrive/Desktop/New folder/New folder/README.md``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: markdown

   One minor aspect of the schema that could be optimised is that cost is repeated in gig_ticket and ticket. In an ideal implementation, it would be better to have ticket.pricetype referencing gig_ticket.pricetype. In this way, the cost of a ticket could be found by joining ticket and gig_ticket, meaning ticket.cost is not required.
   
   Another possible change could be to store the offtime instead of duration in act_gig, as the offtime property is used far more often than duration. This would remove repeatedly recalculating the offtime for each act when performing queries. For checking an act is onstage for less than 2 hours, the duration can still be calculated with offtime - ontime.
   
   For this specific coursework, the members field and the genre fields of act are never used, so it could be removed without affecting any of the query results. In addition, as actname is guaranteed to be unique (from the FAQ), we could also remove the actID field and use actname as the primary key for act, reducing the number of fields used. A similar optimisation can be applied to venuename and venueid.
   
   The cost of an arbitrary ticket/actfee/hirecost may not necessarily be a whole integer, so the type of these fields should be changed to MONEY or NUMERIC instead.
   
   
   Constraint Checks
      - The majority of constraint checking is performed within triggers. For example, when a new act is inserted, the check_act_timings() function will be triggered. This function raises an exception if a constraint is violated, which will be handled in the Java subroutine. In option 2, if an act_gig is invalid, we rollback the data to the last valid state, which is before the new gig was inserted.
   
   Option 1
      - To get the gig line up for a given gigID, option 1 simply joins act, act_gig and gig together, which allows the actname and ontime to be output. To get the offtime for an act, we use offtime = ontime + duration.
      - The results are ordered by ontime ascending.
   
   Option 2
      - In organise_new_gig, the gig's properties are inserted into a new row in the gig table. I created an auxilliary function, get_venue_id_from_name, to get the venue id from the given venue name
      - Then for each act in the gig, the Java subroutine calls insert_new_act_gig, which adds a single act_gig to the table
      - Notably, after the gig and all its corresponding acts have been inserted, the Java option calls check_gig_gaps_less_than_20_mins, to check that no constraints have been violated. This procedure checks that any breaks in the lineup are < 20 mins long and the first act's ontime is within 20 minutes of the gig start time. If one of these constraints are violated, an exception will be thrown, which is handled in a catch clause of the Java function. In this example, the database is rolled back to before the gig was added.
   
   Option 3
      - To book a ticket, first we must check that the gig exists and that the ticket type is valid. Therefore if there are any errors, the database state is as it was
      before the method was called.
      - Then we simply can get the corresponding ticket cost for the given ticket type, and insert a new row into the ticket table.
   
   Option 4
      - To cancel an act, we first check if the act is the headliner of the gig and store this in a boolean variable. We then remove the act_gig with actid and gigID matching the provided IDs. Furthermore, we also make a call to the check_gig_gaps_less_than_20_mins function, because we must ensure that database state is maintained correctly after this act has been cancelled. If we determined that the act is headlining or a check_gig_gaps_less_than_20_mins indicated a constraint was violated (e.g. cancelling act would cause > 20 minute gap, then we know the gig must also be cancelled and we can call cancel_gig().
   
   Option 5
      - We decided to split option 5 into small subroutines that get the required properties to calculate the tickets needed to sell:
        - get_venue_fee_for_gig gets the venue fee for a given gig
        - get_agreed_fees_for_gig gets the agreed act fees for a given gig by summing up all the act fees
        - get_value_of_tickets_sold_for_gig sums all the ticket costs sold so far which match the provided gigID
        - get_standard_ticket_price obtains the standard ticket price for an arbitrary gig
        - get_tickets_needed_to_sell_for_gig calculates the tickets needed to sell to at least cover the venue fees and act fees for a SINGLE gig using the above functions. It uses the following formula to find the tickets still needed to be sold: MAX[CEIL((venue_fee + agreed_fees - tickets_already_sold) / standard_ticket_price), 0]. The max function is included as if the gig has already met the ticket quota to become profitable, then no further tickets need to be sold.
        - get_tickets_needed_to_sell_for_all_gigs will call get_tickets_needed_to_sell_for_gig to find the tickets needed to sell for EVERY gig.
   
   Option 6
      - First get all the headline acts for each gig
      - Then calculate a rank column, which is used for ordering an act based on total ticket sales over the years
      - Then group all the tickets by the actID and year, to get the total sales for an act for each year
      - UNION this with grouping the tickets by just actID, to get the total sales for an act over ALL years.
      - The ordering column calculated previously is added to both sets before the UNION operation, which is used to order the acts by total ticket sales, then by year.
   
   Option 7
      - First join the tickets with act_gig table and act table where the act is a headliner
      - Then get the distinct years where each act has played as a headliner
      - Then Get the NUMBER (i.e count) of distinct years where act has headlined. For instance, if a act has played in 2017, 2018 and 2020, this count would be 3.
      - Using the first CTE, get NUMBER of distinct years where customer has been to a concert with the act headlining.
      - Then perform a LEFT JOIN on the third and fourth CTEs. Importantly, the join must using actname AND the number of concerts given / attended (the count column), to get the act-customer pairs where the customer has been to a concert each calender year. The LEFT JOIN means acts that do not have a customer that has been to a concert each calender year, are still present in the final results table.
   
   Option 8
      - To get the economically feasible gigs, we first CROSS JOIN acts with venues before filtering out non-economically viable gigs in a WHERE clause. In the WHERE clause we use venue.capacity * avg_ticket_price >= act.standardfee + venue.hirecost to only include gigs that would be profitable (assuming all tickets sold). To calculate the number of tickets needed to cover the fees, we used: tickets_required = CEIL((act.standardfee + venue.hirecost) / avg_ticket_price). Then, for ordering the results based on the proportion of tickets that need to be sold, we used added a rank column r = tickets_required/venue.capacity. The query is order by venuename then by descending order of the rank. 
   
